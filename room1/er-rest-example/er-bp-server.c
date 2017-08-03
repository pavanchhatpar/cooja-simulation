/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Erbium (Er) REST Engine example.
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "rest-engine.h"

#if PLATFORM_HAS_BUTTON
#include "dev/button-sensor.h"
#endif

#include <stdio.h>

#define UDP_BP_PORT 9000
#define UDP_RFID_PORT 5678
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "sys/ctimer.h"

#define MAX_PAYLOAD_LEN		30
#ifndef PERIOD
#define PERIOD 60
#endif
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define KEY 12346

struct ll {
   struct ll *next;
   struct ll *prev;
   char nonce[20];
};

/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */
extern resource_t
  res_chunks,
  res_separate,
  res_event;

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

struct ll *nhead = NULL;

static char rfid[10];
static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(res_bp,
         "title=\"Blood Pressure sensor(JSON, Plain TEXT, XML)\";rt=\"Reporter\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset) {

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);
  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "120;80;%s",rfid);

    REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_XML) {
    REST.set_header_content_type(response, REST.type.APPLICATION_XML);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "<sys>120</sys><dias>80</dias><rfid>%s</rfid>", rfid);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'sys': 120, 'dias': 80, 'rfid': '%s'}", rfid);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
}

static int data_received = 0;

static int
isNonceCorrect(char cstr[20]) {
  struct ll *curr = nhead;
  struct ll *tmp;
  int i;
  while(curr != NULL) {
    i = -1;
    while(cstr[++i] != '\0' && curr->nonce[i] != '\0') {
      if(curr->nonce[i] != cstr[i]) {
        break;
      }
    }
    if(cstr[i] == '\0' && curr->nonce[i] == '\0') {
      tmp = curr;
      if(curr->prev != NULL) {
        curr->prev->next = curr->next;
      }
      if(curr->next != NULL) {
        curr->next->prev = curr->prev;
      }
      free(tmp);
      return 1;
    }
    curr = curr->next;
  }
  return 0;
}

static void
save_cnonce(char cstr[20]) {
  struct ll *curr;
  int i = -1;
  if(nhead == NULL) {
    nhead = (struct ll*)malloc(sizeof(struct ll));
    nhead->next = NULL;
    nhead->prev = NULL;
    curr=nhead;
  } else {
    curr = nhead;
    while(curr->next != NULL) {
      curr = curr->next;
    }
    curr->next = (struct ll*)malloc(sizeof(struct ll));
    curr->next->prev = curr;
    curr = curr->next;
    curr->next = NULL;
  }
  while(cstr[++i] != '\0') {
    curr->nonce[i] = cstr[i];
  }
  curr->nonce[i] = '\0';
}

/*---------------------------------------------------------------------------*/
static void
send_cnonce(char *cstr)
{
  //static int seq_id;
  char buf[MAX_PAYLOAD_LEN];
  sprintf(buf, "%s", cstr);
  uip_udp_packet_sendto(client_conn, buf, strlen(buf),
                        &server_ipaddr, UIP_HTONS(UDP_RFID_PORT));
}

/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str, cstr[20], non[20];
  int i=0, j = 0;
  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    switch(str[0]) {
      case 'd':
       
        while(str[++i] != '$') {
          non[i-1] = str[i];
        }
        non[i-1] = '\0';
        if(isNonceCorrect(non)) {
    	  while(str[++i] != '\0') {
      	    rfid[j++] = str[i];
          }
    	  rfid[i] = '\0';
          data_received = 1;
	  printf("RFID tag received: '%s'\n", rfid);
        } else {
          printf("NONCE DID NOT MATCH %s\n", non);
        }
	break;
      default:
        i = -1;
        while(str[++i] != '\0') {
          cstr[i] = (str[i] - '0' + KEY)%10 + '0';
        }
	cstr[i] = '\0';
	send_cnonce(cstr);
	i = -1;
        while(cstr[++i] != '\0') {
          cstr[i] = (cstr[i] - '0' + KEY)%10 + '0';
        }
	cstr[i] = '\0';
	save_cnonce(cstr);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
send_packet(void *ptr)
{
  //static int seq_id;
  char buf[MAX_PAYLOAD_LEN];

  printf("Sending request to ");
  PRINT6ADDR(&server_ipaddr);
  printf(" for nonce\n");
  sprintf(buf, "nSend nonce");
  uip_udp_packet_sendto(client_conn, buf, strlen(buf),
                        &server_ipaddr, UIP_HTONS(UDP_RFID_PORT));
}
/*---------------------------------------------------------------------------*/



PROCESS(er_example_server, "Erbium Example Server");
AUTOSTART_PROCESSES(&er_example_server);

PROCESS_THREAD(er_example_server, ev, data)
{
  static struct etimer periodic;
  static struct ctimer backoff_timer;
  PROCESS_BEGIN();

  PROCESS_PAUSE();

  printf("Start Erbium Example Server\n");
  printf("Start UDP client on  port %d\n", UIP_HTONS(UDP_BP_PORT));
 
  uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x7402, 0x0002, 0x0202);
  client_conn = udp_new(NULL, UIP_HTONS(UDP_RFID_PORT), NULL); 
  if(client_conn == NULL) {
    printf("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(client_conn, UIP_HTONS(UDP_BP_PORT)); 
  
  // Initialize the REST engine. 
  rest_init_engine();

  rest_activate_resource(&res_bp, "sensor/bp");

  etimer_set(&periodic, SEND_INTERVAL);
  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    if(etimer_expired(&periodic) && !data_received) {
      etimer_reset(&periodic);
      ctimer_set(&backoff_timer, SEND_TIME, send_packet, NULL);
    }
#if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      PRINTF("*******BUTTON*******\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

      /* Also call the separate response example handler. */
      res_separate.resume();
    }
    
#endif /* PLATFORM_HAS_BUTTON */
  }                             /* while (1) */

  PROCESS_END();
}

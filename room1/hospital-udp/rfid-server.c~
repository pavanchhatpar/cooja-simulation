/*
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
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"
#include "node-id.h"

#include "net/netstack.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF   ((struct uip_udp_hdr *)&uip_buf[UIP_LLH_LEN + UIP_IPH_LEN])
#define UDP_TEMP_PORT	8765
#define UDP_RFID_PORT	5678
#define UDP_ECG_PORT	8768
#define UDP_GLU_PORT	8771
#define UDP_OXY_PORT	8774
#define UDP_RESP_PORT	8777
#define UDP_BP_PORT     9000

#define UDP_EXAMPLE_ID  190

#define KEY 12346


struct ll {
   struct ll *next;
   struct ll *prev;
   char nonce[20];
};

struct ll *nhead = NULL;
//srand(time(NULL));

static struct uip_udp_conn *server_conn, *server_conn_bp, *server_conn_ecg, *server_conn_glu, *server_conn_oxy, *server_conn_resp;

PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);

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
save_nonce(char cstr[20]) {
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
tcpip_handler(void)
{
  char *appdata;
  char non[20], sdata[20], ndata[20], rfid[3];
  int i, nonce, j, k;
  printf("hello\n");
  sprintf(rfid, "%d", node_id);
  printf("world\n");
  if(uip_newdata()) {
    appdata = (char *)uip_appdata;
    appdata[uip_datalen()] = '\0';
    switch(appdata[0]) {
    	case 'n':
	    nonce = rand() % 7543;
	    if (nonce < 0) {
		nonce *= -1;
	    }
	    sprintf(ndata, "%d", nonce);
	    PRINTF("NONCE sending %s reply to port %d\n", ndata, UIP_UDP_BUF->srcport);
	    if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_TEMP_PORT)) {
	      uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn->ripaddr);
	    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_BP_PORT)) {
	      uip_ipaddr_copy(&server_conn_bp->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn_bp, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn_bp->ripaddr);
	    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_ECG_PORT)) {
	      uip_ipaddr_copy(&server_conn_ecg->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn_ecg, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn_ecg->ripaddr);
	    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_GLU_PORT)) {
	      uip_ipaddr_copy(&server_conn_glu->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn_glu, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn_glu->ripaddr);
	    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_OXY_PORT)) {
	      uip_ipaddr_copy(&server_conn_oxy->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn_oxy, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn_oxy->ripaddr);
	    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_RESP_PORT)) {
	      uip_ipaddr_copy(&server_conn_resp->ripaddr, &UIP_IP_BUF->srcipaddr);
	      uip_udp_packet_send(server_conn_resp, ndata, sizeof(ndata));
	      uip_create_unspecified(&server_conn_resp->ripaddr);
	    }
	    i = -1;
	    while(ndata[++i] != '\0') {
		ndata[i] = (ndata[i] - '0' + KEY) % 10 + '0';
	    }
	    printf("saving nonce %s\n", ndata);
	    save_nonce(ndata);
	    break;
	default:
		i = -1;
		while(appdata[++i] != '\0') {
		  non[i] = appdata[i];
		}
		non[i] = '\0';	
		if(isNonceCorrect(non)) {
		    printf("Request to send data received\n");
		    i = -1;
		    sdata[0] = 'd';
		    while(non[++i] != '\0') {
			sdata[i+1] = (non[i] - '0' + KEY)%10 + '0';
		    }
		    i++;
		    sdata[i] = '$';
		    j = strlen(rfid);
		    k = 0;
		    while(j-- > 0) {
			sdata[++i] = rfid[k++];
		    }	
		    sdata[i+1] = '\0';
		    printf("%s\n", sdata);
		    if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_TEMP_PORT)) {
		      uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn->ripaddr);
		    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_BP_PORT)) {
		      uip_ipaddr_copy(&server_conn_bp->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn_bp, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn_bp->ripaddr);
		    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_ECG_PORT)) {
		      uip_ipaddr_copy(&server_conn_ecg->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn_ecg, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn_ecg->ripaddr);
		    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_GLU_PORT)) {
		      uip_ipaddr_copy(&server_conn_glu->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn_glu, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn_glu->ripaddr);
		    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_OXY_PORT)) {
		      uip_ipaddr_copy(&server_conn_oxy->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn_oxy, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn_oxy->ripaddr);
		    } else if((int)UIP_UDP_BUF->srcport == UIP_HTONS(UDP_RESP_PORT)) {
		      uip_ipaddr_copy(&server_conn_resp->ripaddr, &UIP_IP_BUF->srcipaddr);
		      uip_udp_packet_send(server_conn_resp, sdata, sizeof(sdata));
		      uip_create_unspecified(&server_conn_resp->ripaddr);
		    }
		} else {
			printf("NONCE DID NOT MATCH\n");
		}	
    }
    
  }
}

/*---------------------------------------------------------------------------*/
static void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Server IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
      /* hack to make address "final" */
      if (state == ADDR_TENTATIVE) {
	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t ipaddr;
  //struct uip_ds6_addr *root_if;

  PROCESS_BEGIN();

  PROCESS_PAUSE();

  SENSORS_ACTIVATE(button_sensor);

  PRINTF("UDP server started\n");

#if UIP_CONF_ROUTER
/* The choice of server address determines its 6LoPAN header compression.
 * Obviously the choice made here must also be selected in udp-client.c.
 *
 * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
 * e.g. set Context 0 to aaaa::.  At present Wireshark copies Context/128 and then overwrites it.
 * (Setting Context 0 to aaaa::1111:2222:3333:4444 will report a 16 bit compressed address of aaaa::1111:22ff:fe33:xxxx)
 * Note Wireshark's IPCMV6 checksum verification depends on the correct uncompressed addresses.
 */
  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
//#if 0
/* Mode 1 - 64 bits inline */
  // uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
//#elif 1
/* Mode 2 - 16 bits inline */
//  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
//#else
/* Mode 3 - derived from link local (MAC) address */
  //uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  //uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
//#endif

  /*uip_ds6_addr_add(&ipaddr, 0, ADDR_MANUAL);
  root_if = uip_ds6_addr_lookup(&ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    dag = rpl_set_root(RPL_DEFAULT_INSTANCE,(uip_ip6addr_t *)&ipaddr);
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("created a new RPL dag\n");
  } else {
    PRINTF("failed to create a new RPL DAG\n");
  }*/
#endif /* UIP_CONF_ROUTER */
  
  print_local_addresses();

  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  NETSTACK_MAC.off(1);
//temperature
  server_conn = udp_new(NULL, UIP_HTONS(UDP_TEMP_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));
//bp
  server_conn_bp = udp_new(NULL, UIP_HTONS(UDP_BP_PORT), NULL);
  if(server_conn_bp == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn_bp, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn_bp->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn_bp->lport),
         UIP_HTONS(server_conn_bp->rport));
//ecg
  server_conn_ecg = udp_new(NULL, UIP_HTONS(UDP_ECG_PORT), NULL);
  if(server_conn_ecg == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn_ecg, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn_ecg->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn_ecg->lport),
         UIP_HTONS(server_conn_ecg->rport));
//glucose
server_conn_glu = udp_new(NULL, UIP_HTONS(UDP_GLU_PORT), NULL);
  if(server_conn_glu == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn_glu, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn_glu->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn_glu->lport),
         UIP_HTONS(server_conn_glu->rport));
//oxygen
server_conn_oxy = udp_new(NULL, UIP_HTONS(UDP_OXY_PORT), NULL);
  if(server_conn_oxy == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn_oxy, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn_oxy->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn_oxy->lport),
         UIP_HTONS(server_conn_oxy->rport));
//respiration
server_conn_resp = udp_new(NULL, UIP_HTONS(UDP_RESP_PORT), NULL);
  if(server_conn_resp == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn_resp, UIP_HTONS(UDP_RFID_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn_resp->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn_resp->lport),
         UIP_HTONS(server_conn_resp->rport));

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    } else if (ev == sensors_event && data == &button_sensor) {
      PRINTF("Initiaing global repair\n");
      rpl_repair_root(RPL_DEFAULT_INSTANCE);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

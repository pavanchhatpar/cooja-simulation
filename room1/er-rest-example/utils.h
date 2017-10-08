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

#define UDP_PORT 9000
#define UDP_RFID_PORT 5678
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-udp-packet.h"
#include "net/ip/uip-debug.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "node-id.h"
#define MAX_PAYLOAD_LEN		30
#ifndef PERIOD
#define PERIOD 60
#endif
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

#include "../challenge_response.h"

struct ll {
   struct ll *next;
   struct ll *prev;
   char nonce[6];
};



struct ll *nhead = NULL;

static char rfid[10];

static int data_received = 0;

static int
isRespCorrect(char cstr[6]) {
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
saveResp(char cstr[6]) {
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
static void send_cnonce(char *);


/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  char *str, cstr[6], non[20], temp[4], tmp[20], resStr[6];
  int i=0, j = 0, k, ind;
  long res;
  if(uip_newdata()) {
    str = uip_appdata;
    str[uip_datalen()] = '\0';
    switch(str[0]) {
      case 'd':
        i = -1;
        while(str[++i] != ',') {
          cstr[i-1] = str[i];
        }
        cstr[i-1] = '\0';
        if(isRespCorrect(cstr)) {
    	  while(str[++i] != '\0') {
	    rfid[j++] = decrypt(str[i]);
          }
    	  rfid[i] = '\0';
          data_received = 1;
	  printf("RFID tag received: '%s'\n", rfid);
        } else {
          printf("CHALLENGE RESPONSE FAILED\n");
        }
	break;
      default:
        i = -1;
        res = 1;
        for (j = 0; j < 4; j++) {
          ind = 0;
          k=10; 
          i++;
          while(str[i] != ',' && str[i] != '\0') {
             ind += (int)(str[i] - '0') * k;
             k /= 10;
             i++;
          }
          res *= cr[ind];
        }
        strcpy(tmp, "");
        sprintf(tmp, "%ld", res);
        strcpy(resStr,"");
        strncpy(resStr, tmp, 5);
        resStr[5] = '\0';
        strcpy(tmp, "");
        strcpy(tmp, resStr);
        for(j = 0; j < 5; j++) {
          non[j] = tmp[ord[j]];
        }
        non[j] = ',';
        non[j+1] = '\0';
        res = 1;
        for(i = 0; i < 4; i++) {
          ind = abs(rand() % 49);
          sprintf(temp, "%02d", ind);
          if (i != 3)
             strcat(temp, ",");
          strcat(non, temp);
          res *= cr[ind]; 
        }
        strcpy(tmp, "");
        strcpy(resStr, "");
        sprintf(tmp, "%ld", res);
        strncpy(resStr, tmp, 5);
        resStr[5]='\0';
        strcpy(tmp, resStr);
        for(i = 0; i < 5; i++) {
          resStr[i] = tmp[ord[i]];
        }
        saveResp(resStr);
        printf("Reply for check %s\n", non);
        send_cnonce(non);
    }
  }
}


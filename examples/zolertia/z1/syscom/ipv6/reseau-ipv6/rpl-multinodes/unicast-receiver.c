/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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
 *
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "simple-udp.h"
#include "servreg-hack.h"


#include "dev/leds.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234
#define SERVICE_ID 190

#define SEND_INTERVAL		(10 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_receiver_process, "Unicast receiver example process");
AUTOSTART_PROCESSES(&unicast_receiver_process);
/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)

{
  printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf(" on port %d from port %d with length %d: '%s'\n",
         receiver_port, sender_port, datalen, data);

  //Création des adresses Ip
  uip_ipaddr_t ipaddrSend;	//Adresse Ip de la carte zolertia
  uip_ipaddr_t ipaddrSendLaunchPad;		//Adresse Ip du LaunchPad
  uip_ipaddr_t ipaddrSender = *sender_addr;		//Adresse Ip pour savoir de qui vient le message

  uip_ip6addr(&ipaddrSend, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 0xb);		//Test de reception de la carte zolertia

  uip_ip6addr(&ipaddrSendLaunchPad, 0xfd00, 0, 0, 0, 0x212, 0x4b00, 0xaff, 0x8587);		//Test de reception du LanchPad

  if(ipaddrSend.u16[6] == ipaddrSender.u16[6] && ipaddrSend.u16[5] == ipaddrSender.u16[5])  //Si on reçoit des données de la carte Zolertia on effectue le code
  {
  	printf("Recu de controlleur LED. Nouveaux Status: %d\n",*data);
  	if(*data == 0)  //Si on reçoit un 0 on eteint la led Bleue
  	{
  		leds_off(LEDS_BLUE);
  		printf("Data: %d", data);
  	}
  	else if (*data == 1)  //Si on reçoit un 1 on allume la led Bleue
  	{
  		leds_on(LEDS_BLUE);
  		printf("Data: %d", data);
  	}
  }
  else if(ipaddrSendLaunchPad.u16[6] == ipaddrSender.u16[6] && ipaddrSendLaunchPad.u16[5] == ipaddrSender.u16[5]) //Si on reçoit les données du LaunchPad on effectue le code
  {
  	printf("Recu de controlleur LaunchPad. Nouveaux Status: %d\n",*data);
 	if(*data == 0)    //Si on reçoit un 0 on eteint la led Verte
	{
	   	leds_off(LEDS_GREEN);
	   	printf("Data: %d", data);
	}
	else if (*data == 1)		//Si on reçoit un 1 on allume la led Verte
	{
	   	leds_on(LEDS_GREEN);
	   	printf("Data: %d", data);
	}
  }
  else
  {
  	printf("Recu Unknown\r\n");   //Si l'adresse est inconnue, on l'indique
  }
}
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 0x01);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n");
    }
  }

  return &ipaddr;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_receiver_process, ev, data)
{
 
  PROCESS_BEGIN();

  set_global_address();

  simple_udp_register(&unicast_connection, UDP_PORT, NULL, UDP_PORT, receiver);

  while(1)
  {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

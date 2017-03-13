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

#include "dev/button-sensor.h"

#include "sys/node-id.h"

#include "simple-udp.h"
#include "servreg-hack.h"

#include <stdio.h>
#include <string.h>

#define UDP_PORT 1234
#define SERVICE_ID 190
#define myAdress 2

#define SEND_INTERVAL		(60 * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection unicast_connection;
static uip_ipaddr_t ipaddrLED;
static uip_ipaddr_t ipaddrServ;

static uint8_t LEDStatus = 0;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "Unicast sender example process");
AUTOSTART_PROCESSES(&unicast_sender_process);
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

	  //generate a temp adress
	  uip_ipaddr_t ipaddrSend;
	  uip_ipaddr_t ipaddrSender = *sender_addr;
	  uip_ip6addr(&ipaddrSend, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 2);


	  if(ipaddrSend.u16[6] == ipaddrSender.u16[6] && ipaddrSend.u16[5] == ipaddrSender.u16[5])
	  {
	  	printf("Recu de controlleur LED. Nouveaux Status: %d\n",*data);
	  	if(*data == 0)
	  		LEDStatus=0;
	  	else if (*data == 1)
	  		LEDStatus=1;

	  }
	  else
	  	printf("Les donnes viennent pas du bon sender\n");
}
/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 1);
  uip_ip6addr(&ipaddrLED, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 3);
  uip_ip6addr(&ipaddrServ, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 6);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf("\n%d\n",i);
    }
  }
  uip_ip6addr(&ipaddrLED, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0,uip_ds6_if.addr_list[1].ipaddr.u8[15] + 2 );
  printf("on va envoyer a: ");
  uip_debug_ipaddr_print(&ipaddrLED);
  printf("\n");


}
/*-------------------------111--------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data)
{

  uip_ipaddr_t *addr = &ipaddrLED;
  uip_ipaddr_t *addrServ = &ipaddrServ;

	PROCESS_BEGIN();

	SENSORS_ACTIVATE(button_sensor);

  //servreg_hack_init();

  set_global_address();

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  while(1)
  {

  	PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (data == &button_sensor));

  	if (LEDStatus==0)
  		LEDStatus = 1;
  	else
  		LEDStatus = 0;

		printf("Sending unicast to ");
		uip_debug_ipaddr_print(addr);
		printf("\n");
		printf("LED Status now: %d\n", LEDStatus);
		simple_udp_sendto(&unicast_connection, &LEDStatus, sizeof(LEDStatus) + 1, addr);
		simple_udp_sendto(&unicast_connection, &LEDStatus, sizeof(LEDStatus) + 1, addrServ);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

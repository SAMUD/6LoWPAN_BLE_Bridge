/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Template
 * \author
 *         FSA
 *         NUP
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "dev/leds.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "core/net/ip/simple-udp.h"
#include "sys/etimer.h"
#include "net/ip/uip-debug.h"

 #define UDPClientPort 8888

/*---------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
/* Declaration --------------------------------------------------------------*/
static struct simple_udp_connection Connection;
/*---------------------------------------------------------------------------*/
/* Process ------------------------------------------------------------------*/
static void receiver(struct simple_udp_connection *c,
											const uip_ipaddr_t *sender_addr,
											uint16_t sender_port,
											const uip_ipaddr_t *receiver_addr,
											uint16_t receiver_port,
											const uint8_t *data,
											uint16_t datalen)
{
	leds_toggle(LEDS_BLUE);
	printf("Received data: ID: %i\n",*data);
}


PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();

  static uint8_t i=0;
  static struct etimer et;


  static uip_ipaddr_t AdresseMCast;



  simple_udp_register(&Connection,UDPClientPort,NULL,UDPClientPort,receiver);

  etimer_set(&et, CLOCK_SECOND*5);

  printf("Start process\n");

  while(1)
  {
  	//PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (data == &button_sensor));
  	PROCESS_WAIT_EVENT();

		if(etimer_expired(&et))
		{
			printf("Timer event\n");
			leds_toggle(LEDS_GREEN);
			i=i+1;
			uip_create_linklocal_allnodes_mcast(&AdresseMCast);
			uip_debug_ipaddr_print(&AdresseMCast);
			simple_udp_sendto(&Connection, &i, sizeof(i), &AdresseMCast);
			etimer_reset(&et);
		}





  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

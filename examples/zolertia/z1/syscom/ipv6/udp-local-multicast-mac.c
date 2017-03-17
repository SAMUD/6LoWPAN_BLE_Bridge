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
#include "net/packetbuf.h"
#include "radio.h"
#include "netstack.h"

#define UDPClientPort 8888
#define EXEMPLE_TX_POWER 23
#define EXEMPLE_CHANNEL 11
#define EXEMPLE_PAN_ID 0XBEEF
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

	radio_value_t Channel;
	NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &Channel);

	printf("Received data: ID:%i  Channel:%d  RSSI:%d  LQ:%d\n",*data, Channel,packetbuf_attr(PACKETBUF_ATTR_RSSI),packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY));
}

static void set_radio_default_parameters(void)
{
	NETSTACK_RADIO.set_value(RADIO_PARAM_TXPOWER, EXEMPLE_TX_POWER);
	NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, EXEMPLE_PAN_ID);
	NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, EXEMPLE_CHANNEL);
}


PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();

  static uint8_t i=0;
  static struct etimer et;
  static uip_ipaddr_t AdresseMCast;

  set_radio_default_parameters();
  simple_udp_register(&Connection,UDPClientPort,NULL,UDPClientPort,receiver);

  etimer_set(&et, CLOCK_SECOND*5);

  printf("Start process\n");

  while(1)
  {
  	PROCESS_WAIT_EVENT();

		if(etimer_expired(&et))
		{

			//read radio settings
			radio_value_t Power;
			NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER,&Power);
			radio_value_t PanId;
			NETSTACK_RADIO.get_value(RADIO_PARAM_PAN_ID,&PanId);
			radio_value_t Channel;
			NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL,&Channel);

			printf("Timer event. Sending data TxPower: %d  PAN_ID %d  Channel: %d\n",Power,PanId,Channel);
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

/*
Hello * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup cc26xx-platforms
 * @{
 *
 * \defgroup cc26xx-examples CC26xx Example Projects
 *
 * Example projects for CC26xx-based platforms.
 * @{
 *
 * \defgroup cc26xx-demo CC26xx Demo Project
 *
 *   Example project demonstrating the CC26xx platforms
 *
 *   This example will work for the following board:
 *   - The CC2650 LaunchPad
 *
 *
 *   This is an IPv6/RPL-enabled example. Thus, if you have a border router in
 *   your installation (same RDC layer, same PAN ID and RF channel), you should
 *   be able to ping6 this demo node.
 *
 *   This example also demonstrates CC26xx BLE operation. The process starts
 *   the BLE beacon daemon (implemented in the RF driver). The daemon will
 *   send out a BLE beacon periodically. Use any BLE-enabled application (e.g.
 *   LightBlue on OS X or the TI BLE Multitool smartphone app) and after a few
 *   seconds the cc26xx device will be discovered.
 *
 * - etimer/clock : Every CC26XX_DEMO_LOOP_INTERVAL clock ticks the LED defined
 *                  as CC26XX_DEMO_LEDS_PERIODIC will toggle and the device
 *                  will print out readings from some supported sensors
 *
 * @{
 *
 * \file
 *     Example demonstrating the cc26xx platforms
 */
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "random.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "ti-lib.h"
#include "sys/node-id.h"

#include "simple-udp.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LOOP_INTERVAL       (CLOCK_SECOND * 5)
#define CC26XX_DEMO_LEDS_PERIODIC       LEDS_YELLOW
#define CC26XX_DEMO_LEDS_BUTTON         LEDS_RED
#define CC26XX_DEMO_LEDS_REBOOT         LEDS_ALL
/*---------------------------------------------------------------------------*/
#define UDP_PORT 1234
#define SERVICE_ID 190
#define myAdress 2

#define BOARD_NAME "XADA-BLE"


static struct simple_udp_connection unicast_connection;
static uip_ipaddr_t ipaddrLED;		//IP Adress of the reciving node
static struct etimer et;		//sending Timer

PROCESS(cc26xx_demo_process, "cc26xx demo process");
AUTOSTART_PROCESSES(&cc26xx_demo_process);
/*---------------------------------------------------------------------------*/
static void receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	//In case someone sends some data	
	printf("Data received from ");
	uip_debug_ipaddr_print(sender_addr);
	printf(" on port %d from port %d with length %d: '%s'\r\n",receiver_port, sender_port, datalen, data);
}
/*---------------------------------------------------------------------------*/
static void set_global_address(void)
{
	uip_ipaddr_t ipaddr;
	int i;
	uint8_t state;

	//IP-Adress of Router
	uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 0x01);
	//IP-Adress of receiving node
	uip_ip6addr(&ipaddrLED, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0xC30C, 0, 0, 0x64);

	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

	printf("IPv6 addresses: ");
	for(i = 0; i < UIP_DS6_ADDR_NB; i++)
	{
		state = uip_ds6_if.addr_list[i].state;
		if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED))
		{
			uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
			printf("\r\n%d\n",i);
		}
	}
	
	//Printf te reciving IP-Adress
	printf("on va envoyer a: ");
	uip_debug_ipaddr_print(&ipaddrLED);
	printf("\r\n");
}
/*---------------------------------------------------------------------------*/
static void
get_sync_sensor_readings(void)
{
	int value;

	printf("-----------------------------------------\r\n");

	value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
	printf("Bat: Temp=%d C\r\n", value);

	value = batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT);
	printf("Bat: Volt=%d mV\r\n", (value * 125) >> 5);

	return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(cc26xx_demo_process, ev, data)
{

	static uint8_t LEDStatus = 0;
	static uip_ipaddr_t *addr = &ipaddrLED;

	PROCESS_BEGIN();

	printf("CC26XX uIPv6 and BLE demo\r\n");

	SENSORS_ACTIVATE(batmon_sensor);

	// Init the BLE advertisement
	rf_ble_beacond_config(0, BOARD_NAME);
	rf_ble_beacond_start();

	//Init uIP
	set_global_address();
	simple_udp_register(&unicast_connection, UDP_PORT,NULL, UDP_PORT, receiver);

	etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);

	while(1) {

		PROCESS_YIELD();

		if(ev == PROCESS_EVENT_TIMER)
		{
			if(data == &et)
			{
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
				get_sync_sensor_readings();
				
				//send data with the new LED-Status
				if (LEDStatus==0)
	  				LEDStatus = 1;
				else
	  				LEDStatus = 0;

	  			printf("Sending unicast to ");
	  			uip_debug_ipaddr_print(addr);
	  			printf("\r\n");
	  			printf("LED Status now: %d\r\n", LEDStatus);
	  			simple_udp_sendto(&unicast_connection, &LEDStatus, sizeof(LEDStatus) + 1, addr);
				
				// reset timer
				etimer_set(&et, CC26XX_DEMO_LOOP_INTERVAL);
			}
		}

	}

	PROCESS_END();
}

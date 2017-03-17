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
#include "dev/button-sensor.h"
#include "sys/etimer.h"
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);


/*---------------------------------------------------------------------------*/
/* Declaration --------------------------------------------------------------*/




/*---------------------------------------------------------------------------*/
/* Process ------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{

	static bool Buttonok=0;
	static struct etimer et;

	PROCESS_BEGIN();

  printf("Start process\n");

  etimer_set(&et, CLOCK_SECOND/50);
  SENSORS_ACTIVATE(button_sensor);

  while(1)
  {
  	PROCESS_WAIT_EVENT();

  	if(etimer_expired(&et))
  	{
  		//printf("Timer event\n");
  		leds_toggle(LEDS_ALL);
  		etimer_reset(&et);
  	}
  	else
  	{
  		printf("Button event\n");
			if (Buttonok==false)
					etimer_restart(&et);
			else
			{
					etimer_stop(&et);
					leds_off(LEDS_ALL);
			}
			Buttonok=!Buttonok;
  	}


  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

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
#include "sys/ctimer.h"
#include <stdbool.h>

/*---------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);


/*---------------------------------------------------------------------------*/
/* Declaration --------------------------------------------------------------*/
static struct ctimer et;


/*---------------------------------------------------------------------------*/
/* Process ------------------------------------------------------------------*/
static void timer1 (void *ptr)
{


		static int fastcount=0;

		printf("Timer 1 has expired\n");
		ctimer_reset(&et);
		//toggle leds
		fastcount++;

		switch (fastcount)
		{
			case 1:
				leds_off(LEDS_BLUE);
				leds_off(LEDS_GREEN);
				leds_off(LEDS_RED);
				break;
			case 2:
				leds_on(LEDS_BLUE);
				leds_off(LEDS_GREEN);
				leds_off(LEDS_RED);
				break;
			case 3:
				leds_on(LEDS_BLUE);
				leds_on(LEDS_GREEN);
				leds_off(LEDS_RED);
				break;
			case 4:
				leds_off(LEDS_BLUE);
				leds_on(LEDS_GREEN);
				leds_on(LEDS_RED);
				break;
			case 5:
				leds_off(LEDS_BLUE);
				leds_off(LEDS_GREEN);
				leds_on(LEDS_RED);
				fastcount=0;
				break;
		}
}
PROCESS_THREAD(main_process, ev, data)
{

	bool Buttonok=0;

	PROCESS_BEGIN();

  printf("Start process\n");

  ctimer_set(&et,CLOCK_SECOND/5,timer1,NULL);
  SENSORS_ACTIVATE(button_sensor);

  while(1)
  {
  	PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (data == &button_sensor));

  	printf("Button event\n");
  	if (Buttonok==false)
  			ctimer_restart(&et);
		else
		{
  			ctimer_stop(&et);
  			leds_off(LEDS_BLUE);
				leds_off(LEDS_GREEN);
				leds_off(LEDS_RED);
		}
  	Buttonok=!Buttonok;

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


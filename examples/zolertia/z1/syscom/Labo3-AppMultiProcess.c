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

/*---------------------------------------------------------------------------*/
/* Constants --------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
/* Declaration --------------------------------------------------------------*/
static int LEDLast = 1;
static int templed = 0;
/*---------------------------------------------------------------------------*/
/* Process ------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();

  printf("Start process\n");
  SENSORS_ACTIVATE(button_sensor);



  while(1)
  {
  	PROCESS_WAIT_EVENT_UNTIL((ev == sensors_event) && (data == &button_sensor));

  	while(templed<255)
  	{
  			templed++;
  	}
  	templed=0;

  	switch (LEDLast) {
  	case 1:	leds_on(LEDS_BLUE);
  			break;
  	case 2: leds_on(LEDS_GREEN);
  			break;
  	case 3:	leds_on(LEDS_RED);
  	  	break;
  	case 4: leds_off(LEDS_BLUE);
  	  	break;
  	case 5:	leds_off(LEDS_GREEN);
  	  			break;
  	case 6: leds_off(LEDS_RED);
  					LEDLast=0;
  	  		break;
  	};
  	LEDLast++;

  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

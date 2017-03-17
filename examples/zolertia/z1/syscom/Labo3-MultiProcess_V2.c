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
#include <stdbool.h>
#include "sys/etimer.h"

/*---------------------------------------------------------------------------*/

static process_event_t toggle_event;
static process_event_t color_event;

static struct etimer et;

/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
PROCESS(toggle_process, "Toggle process");
PROCESS(color_process, "Color process");
AUTOSTART_PROCESSES(&main_process, &toggle_process,&color_process);
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Process —----------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{

	PROCESS_BEGIN();
  printf("Start process\n");
  SENSORS_ACTIVATE(button_sensor);
  leds_off(LEDS_ALL);
  etimer_set(&et, CLOCK_SECOND/2);

  static unsigned char CurrentLED = LEDS_BLUE;

  while(1)
  {
    PROCESS_WAIT_EVENT();

    if (etimer_expired(&et))
    {
    	etimer_reset(&et);
    	process_post(&toggle_process,toggle_event,&CurrentLED);
    	printf("main process | Timer expired      ");
    	printf("Passed Pointer: %p\n",&CurrentLED);
    }

    if((ev == sensors_event) && (data == &button_sensor))
    {
      process_post(&color_process, color_event, &CurrentLED);
      printf("main process | Button pressed     ");
      printf("Passed Pointer: %p\n",&CurrentLED);
    }

  }
  PROCESS_END();
}

PROCESS_THREAD(toggle_process, ev,data)
	{
  PROCESS_BEGIN();
  while(1)
  {

    PROCESS_WAIT_EVENT_UNTIL(ev == toggle_event);
    unsigned char* color=data;
    leds_toggle(*color);
    printf("toggle_process | Toggled LED: \%d\n",*color);

    //turning off the other leds
    if(*color != LEDS_BLUE )
    	leds_off(LEDS_BLUE);
    if(*color != LEDS_RED )
        	leds_off(LEDS_RED);
    if(*color != LEDS_GREEN )
        	leds_off(LEDS_GREEN);

  }
  PROCESS_END();
}

PROCESS_THREAD(color_process, ev, data){
  PROCESS_BEGIN();
  while(1)
  {

    PROCESS_WAIT_EVENT_UNTIL(ev == color_event);
    unsigned char *color=data;
    printf("color_process | Changing color from \%d\n",*color);
    switch(*color)
    {
    case LEDS_GREEN:
    	*color=LEDS_BLUE;
    	break;
    case LEDS_BLUE:
    	*color=LEDS_RED;
    	break;
    default:
    	*color=LEDS_GREEN;
    	break;
    }
  }
  PROCESS_END();
}



/*---------------------------------------------------------------------------*/

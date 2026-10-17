/*
 * main.c
 *
 *
 *  Created on: 20.02.2017
 *      Author: Eagle
 */

#include <AVR/interrupt.h>
#include "bool.h"

#include "rcc.h"
#include "tim.h"
#include "adc.h"

#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "SW_INPUT/input.h"
#include "SW_LEDLIGHT/sw_ledlight_manage.h"


int main(void) {
	RCC_Clock_Init();

	sw_soft_timer_init();
	sw_led_debug_init();

	LEDLIGHT_init();

//	ADC_init();
//	adc_event_cb_reg( LEDLIGHT_check_battery );

//	sw_led_on();

	sei();
//	LEDLIGHT_mode_set( LED_MODE_ALL_OFF );
//	LEDLIGHT_power_down();	// After reset
//	DEBUG_blink_start( LED_BLINK_INFINITE, 1000 );

	while (1) {
		INPUT_event_execute();
		ADC_EVENT();
		DEBUG_BLINK_EVENT();

		if ( softTimer1 == 0 ) {
			softTimer1 = 500;
//			sw_led_blink();
//			LEDLIGHT_more_light();
		}
	}
}


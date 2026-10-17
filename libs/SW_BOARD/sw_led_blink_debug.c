/*
 * led_blink.c
 *
 *  Created on: 05.07.2017
 *      Author: simon
 */
#include <avr/interrupt.h>

#include "sw_mcu_conf.h"

#include "gpio.h"

#include "SW_TIMERS/sw_soft_timers.h"

#include "sw_led_blink_debug.h"

enum { _led_blinking, _led_not_blinking };

void sw_led_on( void ) {
	gpio_pin_HI( LED_DIAG_PORT, LED_DIAG_PIN );
}
void sw_led_off( void ) {
	gpio_pin_LO( LED_DIAG_PORT, LED_DIAG_PIN );
}

void sw_led_blink( void ) {
	gpio_pin_XOR( LED_DIAG_PORT, LED_DIAG_PIN );
}

void sw_led_debug_init( void ) {
#ifdef LED_DIAG_PORT
	gpio_pin_cfg( LED_DIAG_PORT, LED_DIAG_PIN, gpio_mode_output_PP );
	sw_led_off();
#endif
#ifdef	DEBUG_PIN0
	gpio_pin_cfg( DEBUG_PORT0, DEBUG_PIN0, gpio_mode_output );
	gpio_pin_LO ( DEBUG_PORT0, DEBUG_PIN0);

#endif
#ifdef	DEBUG_PIN1
	gpio_pin_cfg(DEBUG_PORT1, DEBUG_PIN1, gpio_mode_output );
	gpio_pin_LO( DEBUG_PORT1, DEBUG_PIN1 );
#endif
#ifdef	DEBUG_PIN2
	gpio_pin_cfg( DEBUG_PORT2, DEBUG_PIN2, gpio_mode_output );
#endif
}

static uint8_t blinkCounter = 0;
static uint8_t blinkDelay 	= 0;
void DEBUG_blink_start( uint8_t repeat, uint16_t delay ) {
	sw_led_off();
	blinkCounter = 2*repeat;
	blinkDelay = delay;
}
void DEBUG_blink_stop(void) {
	blinkCounter = 0;
}

int8_t DEBUNG_blink_get_status(void) {
	if (blinkCounter == 0) 	return (int8_t)_led_not_blinking;
	else					return (int8_t)_led_blinking;
}

void DEBUG_BLINK_EVENT(void) {
	if ( blinkCounter == 0 || softTimer3 != 0 ) return;	// Nic nie robimy jeśli
	else {
		softTimer3 = blinkDelay;
		sw_led_blink();
		blinkCounter--;
	}
}

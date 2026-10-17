/*
 * keyboard.c
 *
 *  Created on: 12 sie 2021
 *      Author: simon
 */

#include "sw_mcu_conf.h"

#include "gpio.h"

#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"

#include "keyboard.h"


enum {
	stateFALL = 1, stateRISE = 2, stateLOW = 3, stateHIGH = 4
};
enum {
	keyRELEASED = 0, keyPRESSED = 1
};
typedef enum {
	action_idle, action_debounce, action_check, action_react, action_repeat
} KEY_ACTION_t;

// ---------------------- Variables and constants -----------------------
static KEYB_OUT_t	keyboard;
static KEY_ACTION_t keyAction = action_idle;

static uint8_t repeatCounter 	 = 0;
static uint8_t speedPressCounter = 0;

static uint16_t timeFallRise  	 = 0;	// Time: Fall and Rise
static uint16_t timeFall2Rise 	 = 0;	// Time: first Fall and last Rise

//static const KEYB_IN_t KeysIN[ NUM_KEYS ] = {
static const KEYB_IN_t KeysIN[] = {
#ifdef BTN1_PORT
	{ BTN1_PORT, BTN1_PIN },
#endif
#ifdef BTN2_PORT
	{ BTN2_PORT, BTN2_PIN },
#endif
#ifdef BTN3_PORT
	{ BTN3_PORT, BTN3_PIN }
#endif
};
// ----------------------------------------------------------------------


static INLINE void machine_state_reset(void) {
	keyboard.keyEvent = NO_EVENT;
	keyboard.keyCoded = NO_KEY;
}

/***************************** Detect slopes ****************************/
static INLINE uint8_t analyze_slope( uint8_t state, uint8_t lastState ) {
	if (state == keyPRESSED  && lastState == keyRELEASED) state = stateFALL;
	else
	if (state == keyPRESSED  && lastState == keyPRESSED)  state = stateLOW;
	else
	if (state == keyRELEASED && lastState == keyPRESSED)  state = stateRISE;
	else
	if (state == keyRELEASED && lastState == keyRELEASED) state = stateHIGH;

	switch ( state ) {
		case stateFALL:
			timeFallRise = millis();
			if (speedPressCounter++ == 0) {
				timeFall2Rise = millis();
			}
			break;
		case stateRISE:
			timeFallRise = millis() - timeFallRise;
			if ( speedPressCounter++ == (EDGE_COUNTER - 1) ) {
				timeFall2Rise = millis() - timeFall2Rise;
			}
			break;
		default: break;
	}
	return state;
}
/************************************************************************/

/***************************** Edge analyzing ***************************/
static INLINE void analyze_state_fall(void) {
	if ( keyAction == action_idle ) {
		keyAction 	  = action_debounce;
		debounceTimer = DEBOUNCE_TIME;
	}
}
/************************************************************************/
/************************** Table state analyzing ***********************/
static KEY_EVENT_t INLINE analyze_state_low(void) {
	KEY_EVENT_t keyEvent = NO_EVENT;

	switch ( keyAction ) {
		case action_debounce:
			if ( debounceTimer == 0 ) {
				keyAction 	  = action_check;
				debounceTimer = SHORT_TIME_MAX;
			}
#if DEBUG
			gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 1 );
#endif
			break;
		case action_check:
//			if ( debounceTimer > MEDIUM_TIME_MIN ) {
			if ( debounceTimer == 0 ) {
				keyAction 	  = action_repeat;
				debounceTimer = MEDIUM_TIME_MAX;
				repeatCounter = 0;
#if DEBUG
				gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 2 );
#endif
			}
#if DEBUG
			gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 2 );
#endif
			break;
		case action_repeat:
			if ( debounceTimer == 0 ) {
				debounceTimer = REPEAT_TIME;
				repeatCounter++;
				if ( repeatCounter == VLONG_TIME_CNT ) {
					keyEvent = VLONG_PRESS;
				} else {
					keyEvent = KEY_REPEAT;
				}
			}
//			gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 3 );
			break;
		default:break;
	}
	return keyEvent;
}
/***************************** Edge analyzing ***************************/
static void INLINE analyze_state_rise(void) {
	if ( keyAction == action_check || keyAction == action_repeat ) {
		debounceTimer = DEBOUNCE_TIME;
	}
}
/************************************************************************/
static KEY_EVENT_t INLINE analyze_state_high(void) {
	KEY_EVENT_t keyEvent = NO_EVENT;

	switch ( keyAction ) {
		case action_check:
			if ( debounceTimer == 0 ) {
				if ( timeFallRise > SHORT_TIME_MIN ) {
					speedPressCounter = 0;
					keyEvent = SHORT_PRESS;
				} else 	{
					if ( speedPressCounter == EDGE_COUNTER ) {
						if ( timeFall2Rise < DOUBLE_PRESS_TIME ) {
							keyEvent = DOUBLE_PRESS;
						}
						speedPressCounter = 0;
					}
				}
				keyAction = action_idle;
			}
//			gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 4 );
			break;
		case action_repeat:
			if ( debounceTimer == 0 ) {
				if ( repeatCounter == 0 ) {
					keyEvent = MEDIUM_PRESS;
				}
				speedPressCounter = 0;
				keyAction = action_idle;
			}
//			gpio_pin_IMPULSES( DEBUG_PORT0, DEBUG_PIN0, 5 );
			break;
		default: break;
	}
	return keyEvent;
}
/************************************************************************/

/********************** Main machine state analyzing ********************/
static KEY_EVENT_t is_key_pressed( void ) {
	static  uint8_t  	lastState = keyRELEASED;
			KEY_EVENT_t keyEvent  = NO_EVENT;

	uint8_t pressKeyCheck = sw_get_keys_simple();

	if (pressKeyCheck) {
		keyboard.keyCoded = pressKeyCheck;
		pressKeyCheck = keyPRESSED;
	} else {
		pressKeyCheck = keyRELEASED;
	}
	uint8_t slope = analyze_slope( pressKeyCheck, lastState );

	switch ( slope ) {
		case stateFALL:
			analyze_state_fall();
			break;
		case stateLOW:
			keyEvent = analyze_state_low();
			break;
		case stateRISE:
			analyze_state_rise();
			break;
		case stateHIGH:
			keyEvent = analyze_state_high();
			break;
		default:
			break;
	}
	lastState = pressKeyCheck;
	if (keyboard.keyCoded)return keyboard.keyEvent = keyEvent;
	else  		 		  return NO_EVENT;
}
/************************************************************************/

/************************ Public functions ******************************/

// ---------------------- Main init function -----------------------------
void KEYBOARD_init(void) {
	uint8_t keyNo = sizeof( KeysIN ) / sizeof( KEYB_IN_t );
	for ( uint8_t i = 0; i < keyNo; i++ ) {
		gpio_pin_cfg( KeysIN[ i ].port, KeysIN[ i ].pin, gpio_mode_in_PU );
	}
	machine_state_reset();
}
// -----------------------------------------------------------------------

KEYB_OUT_t * keyboard_ptr(void) {
	return &keyboard;
}

void SW_KEYBOARD_EVENT( KEYB_OUT_t * keyboard_ptr ) {
	volatile KEY_EVENT_t keyEvent = is_key_pressed();

	if ( keyEvent == NO_EVENT ) {
		keyboard_ptr->keyEvent = NO_EVENT;
		return;
	}
	*keyboard_ptr = keyboard;
	if ( keyEvent != KEY_REPEAT) {
		machine_state_reset();
	}
}
/************************************************************************/


#if TEST_KEYBOARD
void keyboard(void) {
	static int8_t counter1, counter2, counter3;
	if ( keyboard_ptr()->shortPRESS == true ) {
		sw_led_start_blinking( 2, 100 );
		TEXT_display_float( 0, 0,  ++counter1,  	&TextX );
		sw_ssd1306_display();
	} else
	if ( keyboard_ptr()->mediumPRESS == true ) {
		TEXT_display_float( 0, 16,  ++counter2,  	&TextY );
		sw_ssd1306_display();
	} else
	if ( keyboard_ptr()->doublePRESS == true ) {
		TEXT_display_float( 0, 32,  ++counter3,  	&TextZ );
		sw_ssd1306_display();
	} else
	if ( keyboard_ptr()->veryLongPress == true ) {
		counter3 = 0; counter2 = 0; counter1 = 0;
		TEXT_display_float( 0,  0,  counter1,  	&TextX );
		TEXT_display_float( 0, 16,  counter2,  	&TextY );
		TEXT_display_float( 0, 32,  counter3,  	&TextZ );
		sw_led_start_blinking( 3, 100 );
		sw_ssd1306_display();
	} else
	if ( keyboard_ptr()->keyREPEAT == true ) {
		sw_led_on();
		TEXT_display_float( 0, 0,  ++counter1,  	&TextX );
		sw_ssd1306_display();
	}
}
#endif


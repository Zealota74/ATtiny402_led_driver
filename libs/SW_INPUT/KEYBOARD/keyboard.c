/*
 * keyboard.c
 */
#include "sw_mcu_conf.h"

#include "gpio.h"
#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "keyboard.h"

enum { stateFALL = 1, stateRISE = 2, stateLOW = 3, stateHIGH = 4 };
enum { keyRELEASED = 0, keyPRESSED = 1 };
typedef enum {
	action_idle, action_debounce, action_check, action_repeat
} KEY_ACTION_t;

// ZMIANA: dostęp do timera w jednym miejscu, przy przejściu na timery funkcyjne zmieniasz tylko te dwa makra
#define KEY_TIMER_START(ms)   ( debounceTimer = (ms) )
#define KEY_TIMER_DONE()      ( debounceTimer == 0 )
#define DOUBLE_WINDOW_TIME  ( DOUBLE_PRESS_TIME - DEBOUNCE_TIME )

// ---------------------- Variables and constants -----------------------
static KEYB_OUT_t	keyboard;
static KEY_ACTION_t keyAction = action_idle;

static uint8_t  repeatCounter     = 0;
static uint8_t  speedPressCounter = 0;
static uint16_t pressStart        = 0;	// ZMIANA: początek przyjętego wciśnięcia
static uint16_t timeFallRise      = 0;	// czas trwania wciśnięcia
static uint8_t  lastState         = keyRELEASED;

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

// ======================================================================
static INLINE uint16_t double_window_time(void) {
	if ( DOUBLE_PRESS_TIME > DEBOUNCE_TIME ) {
		return DOUBLE_PRESS_TIME - DEBOUNCE_TIME;
	}
	return SYSTICK_MS;      // zabezpieczenie: DOUBLE_PRESS_TIME ustawiony za mały
}
// ----------------------------------------------------------------------
static INLINE void machine_state_reset(void) {
	keyboard.keyEvent = NO_EVENT;
	keyboard.keyCoded = NO_KEY;
}
// ----------------------------------------------------------------------
static INLINE uint8_t analyze_slope( uint8_t state, uint8_t last ) {
	if (state == keyPRESSED  && last == keyRELEASED) return stateFALL;
	if (state == keyPRESSED  && last == keyPRESSED)  return stateLOW;
	if (state == keyRELEASED && last == keyPRESSED)  return stateRISE;
	return stateHIGH;
}
// ----------------------------------------------------------------------
static INLINE void analyze_state_fall(void) {
	if ( keyAction == action_idle ) {
		keyAction     = action_debounce;
		KEY_TIMER_START(DEBOUNCE_TIME);
		pressStart    = millis();
	}
}
// ----------------------------------------------------------------------
static KEY_EVENT_t INLINE analyze_state_low(void) {
	KEY_EVENT_t keyEvent = NO_EVENT;

	switch ( keyAction ) {
		case action_debounce:
			if (  KEY_TIMER_DONE() ) {
				keyAction     = action_check;
				KEY_TIMER_START(SHORT_TIME_MAX);
			}
			break;
		case action_check:
			if (  KEY_TIMER_DONE() ) {
				keyAction     = action_repeat;
				KEY_TIMER_START(MEDIUM_TIME_MAX);
				repeatCounter = 0;
			}
			break;
		case action_repeat:
			if (  KEY_TIMER_DONE() ) {
				KEY_TIMER_START(REPEAT_TIME);
				repeatCounter++;
				if ( repeatCounter == VLONG_TIME_CNT ) {
					keyEvent = VLONG_PRESS;
				} else {
					keyEvent = KEY_REPEAT;
				}
			}
			break;
		default: break;
	}
	return keyEvent;
}
// ------------------------------------------------------------------------
static void INLINE analyze_state_rise(void) {
	if ( keyAction == action_check || keyAction == action_repeat ) {
		KEY_TIMER_START(DEBOUNCE_TIME);
		timeFallRise  = millis() - pressStart;
	}
}
// ------------------------------------------------------------------------
static KEY_EVENT_t INLINE analyze_state_high(void) {
	KEY_EVENT_t keyEvent = NO_EVENT;

	switch ( keyAction ) {
		case action_debounce:
			keyAction = action_idle;
			break;
		case action_check:
			if (  KEY_TIMER_DONE() ) {
				if ( timeFallRise >= SHORT_TIME_MIN && timeFallRise <= SHORT_TIME_MAX ) {
					if ( speedPressCounter == 0 ) {
						// pierwsze puszczenie: otwórz okno na drugie kliknięcie
						speedPressCounter = 1;
						// ZMIANA: DOUBLE_PRESS_TIME = maksymalny odstęp od puszczenia
						KEY_TIMER_START( double_window_time() );
						keyAction = action_idle;
					} else {
						keyEvent = DOUBLE_PRESS;
						speedPressCounter = 0;
						keyAction = action_idle;
					}
				} else {
					speedPressCounter = 0;
					keyAction = action_idle;
				}
			}
			break;
		case action_repeat:
			if (  KEY_TIMER_DONE() ) {
				if ( repeatCounter == 0 && timeFallRise >= MEDIUM_TIME_MIN ) {
					keyEvent = MEDIUM_PRESS;
				}
				speedPressCounter = 0;
				keyAction = action_idle;
			}
			break;
		default: break;
	}
	// okno double press minęło bez drugiego kliknięcia -> SHORT_PRESS
	if ( speedPressCounter == 1 && keyAction == action_idle ) {
		if (  KEY_TIMER_DONE() ) {
			keyEvent = SHORT_PRESS;
			speedPressCounter = 0;
		}
	}
	return keyEvent;
}
// ========================================================================

// ==================== Main machine state analyzing ======================
static KEY_EVENT_t keyboard_step( void ) {
	KEY_EVENT_t keyEvent = NO_EVENT;

	uint8_t pressKeyCheck = key_scan();

	if (pressKeyCheck) {
		keyboard.keyCoded = pressKeyCheck;
		pressKeyCheck = keyPRESSED;
	} else {
		pressKeyCheck = keyRELEASED;
	}
	uint8_t slope = analyze_slope( pressKeyCheck, lastState );

	switch ( slope ) {
		case stateFALL: analyze_state_fall();               break;
		case stateLOW:  keyEvent = analyze_state_low();     break;
		case stateRISE: analyze_state_rise();               break;
		case stateHIGH: keyEvent = analyze_state_high();    break;
		default: break;
	}
	lastState = pressKeyCheck;
	if (keyboard.keyCoded) return keyboard.keyEvent = keyEvent;
	else                   return NO_EVENT;
}
// ========================================================================

// ========================= Public functions =============================
void KEYBOARD_init(void) {
	uint8_t keyNo = sizeof( KeysIN ) / sizeof( KEYB_IN_t );
	for ( uint8_t i = 0; i < keyNo; i++ ) {
		gpio_pin_cfg( KeysIN[ i ].port, KeysIN[ i ].pin, gpio_mode_in_PU );
	}
	machine_state_reset();
}
// -----------------------------------------------------------------------
void SW_KEYBOARD_EVENT( KEYB_OUT_t * out  ) {
	KEY_EVENT_t keyEvent = keyboard_step();

	if ( keyEvent == NO_EVENT ) {
		out->keyEvent = NO_EVENT;
		return;
	}
	*out  = keyboard;
	if ( keyEvent != KEY_REPEAT ) {
		machine_state_reset();
	}
}
// -----------------------------------------------------------------------
void KEYBOARD_reset(void) {
	keyAction         = action_idle;
	speedPressCounter = 0;
	repeatCounter     = 0;
	pressStart        = 0;
	timeFallRise      = 0;
	lastState         = keyRELEASED;
	debounceTimer     = 0;
	machine_state_reset();
}
// ========================================================================
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


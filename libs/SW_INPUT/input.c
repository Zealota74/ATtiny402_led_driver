
#include "sw_mcu_conf.h"
#include <avr/interrupt.h>

#include "gpio.h"

#include "KEYBOARD/keyboard.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "SW_BOARD/sw_led_blink_debug.h"

#include "input.h"

// ZMIANA: współdzielone z ISR ticka (INPUT_event_get), więc volatile
static volatile int8_t      inputEventFlag = INPUT_NO_EVENT;
static volatile KEYB_OUT_t  keyboardOut;      // ZMIANA: nazwa nie udaje wskaźnika
static COMMAND_FUN_t        InputEvents[ EVENT_NUMBER ];

// ---------------------------- Private functions -----------------------------
static void INPUT_EVENTS_cb_exec( INPUT_EVENT_t event ) {
	if ( InputEvents[ event ] ) {
		InputEvents[ event ]();
	}
}

static void INPUT_keyboard_analyze( KEY_EVENT_t presType, uint8_t keyNumber ) {
	if ( keyNumber != KEY1 ) {              // ZMIANA: jedno miejsce, także dla KEY_REPEAT (wcześniej ignorował numer)
		return;
	}
	switch ( presType ) {
		case SHORT_PRESS:   INPUT_EVENTS_cb_exec( EVENT_KEY1_SHORT_PRESS );  break;
		case MEDIUM_PRESS:  INPUT_EVENTS_cb_exec( EVENT_KEY1_MEDIUM_PRESS ); break;
		case VLONG_PRESS:   INPUT_EVENTS_cb_exec( EVENT_KEY1_VLONG_PRESS );  break;
		case DOUBLE_PRESS:  INPUT_EVENTS_cb_exec( EVENT_KEY1_DOUBLE_PRESS ); break;
		case KEY_REPEAT:    INPUT_EVENTS_cb_exec( EVENT_KEY1_REPEAT_PRESS ); break;
		default: break;
	}
}

// ---------------------------- Public functions ------------------------------
void INPUT_EVENTS_cb_reg( INPUT_EVENT_t event, COMMAND_FUN_t input_event_cb ) {
	if ( input_event_cb && event < EVENT_NUMBER ) {   // ZMIANA: ochrona przed zapisem poza tablicę
		InputEvents[ event ] = input_event_cb;
	}
}

void INPUT_init( void ) {
	KEYBOARD_init();
}

// Wołane z ticka (ISR)
void INPUT_event_get( void ) {
	KEYB_OUT_t tmp;
	SW_KEYBOARD_EVENT( &tmp );
	if ( tmp.keyEvent != NO_EVENT ) {
		keyboardOut.keyEvent = tmp.keyEvent;
		keyboardOut.keyCoded = tmp.keyCoded;
		inputEventFlag       = INPUT_KEYB_EVENT;
	}
}

// Wołane z pętli głównej
void INPUT_event_execute( void ) {
	KEY_EVENT_t ev  = NO_EVENT;
	uint8_t     key = NO_KEY;
	uint8_t     flag;

	// ZMIANA: kopia pod cli(), żeby ISR nie nadpisał danych w trakcie analizy
	// (SREG zachowany, bo funkcja może być wołana także z callbacku)
	uint8_t sreg = SREG;
	cli();
	flag = inputEventFlag;
	if ( flag == INPUT_KEYB_EVENT ) {
		ev  = keyboardOut.keyEvent;
		key = keyboardOut.keyCoded;
		inputEventFlag = INPUT_NO_EVENT;
	}
	SREG = sreg;

	if ( flag == INPUT_KEYB_EVENT ) {
		INPUT_keyboard_analyze( ev, key );   // callback poza sekcją krytyczną
	}
}

// Wołać po każdej przerwie w ciągłości czasu (uśpienie, długa blokada)
void INPUT_reset( void ) {
	uint8_t sreg = SREG;
	cli();
	KEYBOARD_reset();
	keyboardOut.keyEvent = NO_EVENT;
	keyboardOut.keyCoded = NO_KEY;
	inputEventFlag       = INPUT_NO_EVENT;
	SREG = sreg;
}

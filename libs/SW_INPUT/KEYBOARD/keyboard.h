/*
 * keyboard.h
 *
 *  Created on: 12 sie 2021
 *      Author: simon
 */

#ifndef SRC_LIBS_KEYBOARD_H_
#define SRC_LIBS_KEYBOARD_H_

#include "stdint.h"

// ------------------ Constant definitions -------------------
// Wszystkie czasy w ms, rozdzielczość = SYSTICK_MS (10 ms)
#define DEBOUNCE_TIME       30         // ms
#define SHORT_TIME_MIN       0         // ms
#define SHORT_TIME_MAX     150         // ms
#define MEDIUM_TIME_MIN    500         // ms, nieużywane
#define MEDIUM_TIME_MAX    700         // ms
#define DOUBLE_PRESS_TIME  200         // ms, max odstęp od puszczenia do kolejnego wciśnięcia
#define REPEAT_TIME        100         // ms
#define VLONG_TIME_CNT      20         // liczba okresów REPEAT_TIME

// Czas od wciśnięcia (przy obecnych ustawieniach):
//   0 - 150 ms   krótkie kliknięcie (SHORT / DOUBLE)
//   150 - 500 ms brak zdarzenia
//   500 - 880 ms MEDIUM_PRESS przy puszczeniu
//   > 880 ms     KEY_REPEAT co REPEAT_TIME, VLONG_PRESS po VLONG_TIME_CNT okresach

#define NO_KEY				0
#define KEY1				1
#define KEY2				2
#define KEY3				3

// -----------------------------------------------------------
typedef enum {
	NO_EVENT 	= 0, SHORT_PRESS  = 1, MEDIUM_PRESS = 2, LONG_PRESS = 3,
	VLONG_PRESS = 4, DOUBLE_PRESS = 5, KEY_REPEAT 	= 6
} KEY_EVENT_t;

// ------------------- Types definitions ---------------------
typedef struct {
	KEY_EVENT_t keyEvent;
	uint8_t 	keyCoded;
//	void ( *keyboard_callback )(void);
} KEYB_OUT_t;

typedef struct {
	GPIO_TypeDef * port;
	T_GPIO_PIN 	   pin;
} KEYB_IN_t;
// ----------------------------------------------------------

#define KEYS_SIMPLE
//#define KEYS_MULTI
//#define KEYS_MATRIX


#ifdef 	KEYS_SIMPLE
//#	define NUM_KEYS				3
#	define BTN1_PORT			KEY_PORT
#	define BTN1_PIN				KEY_PIN
//#	define BTN2_PORT			&PORTB
//#	define BTN2_PIN				 PA11
//#	define BTN3_PORT			&PORTB
//#	define BTN3_PIN				 PA12


/****************************** Few keys manage *****************************/
static inline uint8_t sw_get_keys_simple( void ) {
#ifdef BTN1_PORT
	if ( gpio_pin_READ( BTN1_PORT, BTN1_PIN ) == 0 ) return 1;
#endif
#ifdef BTN2_PORT
	if ( gpio_pin_READ( BTN2_PORT, BTN2_PIN ) == 0 ) return 2;
#endif
#ifdef BTN3_PORT
	if ( gpio_pin_READ( BTN3_PORT, BTN3_PIN ) == 0 ) return 3;
#endif
	return 0;
}
// ----------------------------------------------------------------------------
// ZMIANA: skaner oddzielony od maszyny gestów (przy matrycy/ADC podmieniasz tylko to)
static INLINE uint8_t key_scan(void) {
	return sw_get_keys_simple();           // 0 = brak klawisza, 1..N = numer
}
/******************************************************************************/
#endif

#ifdef KEYS_MULTI
#	define NUM_KEYS	10
#endif



// ----------------- public declarations --------------------
extern void KEYBOARD_init(void);
extern void SW_KEYBOARD_EVENT( KEYB_OUT_t * keyboard_ptr );
extern void KEYBOARD_reset(void);

#endif /* SRC_LIBS_KEYBOARD_H_ */

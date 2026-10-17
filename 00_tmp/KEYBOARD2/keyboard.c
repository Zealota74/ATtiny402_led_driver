/*
 * keyboard.c
 *
 * Obsługa przycisku z debounce, klasyfikacją czasu przytrzymania,
 * double-click oraz powtarzaniem.
 *
 * Założenie: SW_KEYBOARD_EVENT() jest wywoływane regularnie (np. co 10 ms)
 * z kontekstu, w którym można bezpiecznie wywołać millis().
 */
#include <stdint.h>
#include <stddef.h>
#include "sw_mcu_conf.h"

#include "gpio.h"

#include "SW_TIMERS/sw_soft_timers.h"
#include "keyboard.h"

enum {
	KEY_RELEASED = 0, KEY_PRESSED = 1
};

typedef enum {
	CLICK_IDLE = 0, CLICK_WAIT_SECOND, CLICK_SECOND_DOWN
} CLICK_STATE_t;

typedef struct {
	KEY_EVENT_t event;
	uint8_t key;
} QUEUED_EVENT_t;

#define EVENT_QUEUE_SIZE 4U

static KEYB_OUT_t keyboard;
static uint8_t stablePressed = KEY_RELEASED;
static uint8_t rawCandidate = KEY_RELEASED;
static uint32_t candidateSince = 0U;
static uint32_t pressStart = 0U;
static uint32_t nextRepeatAt = 0U;
static uint8_t currentPressKey = NO_KEY;
static uint16_t repeatCounter = 0U;
static bool longEventSent = false;

static CLICK_STATE_t clickState = CLICK_IDLE;
static uint32_t firstReleaseTime = 0U;
static uint8_t firstClickKey = NO_KEY;

static QUEUED_EVENT_t eventQueue[ EVENT_QUEUE_SIZE ];
static uint8_t queueHead = 0U;
static uint8_t queueTail = 0U;
static uint8_t queueCount = 0U;

static const KEYB_IN_t KeysIN[ ] = {
#ifdef BTN1_PORT
		{ BTN1_PORT, BTN1_PIN },
#endif
#ifdef BTN2_PORT
    { BTN2_PORT, BTN2_PIN },
#endif
#ifdef BTN3_PORT
    { BTN3_PORT, BTN3_PIN },
#endif
		};

static void queue_event( KEY_EVENT_t event, uint8_t key ) {
	if ( event == NO_EVENT ) {
		return;
	}

	/* Przy normalnym użyciu kolejka nie powinna się zapełnić.
	 Jeśli się zapełni, zachowujemy istniejące zdarzenia. */
	if ( queueCount >= EVENT_QUEUE_SIZE ) {
		return;
	}

	eventQueue[ queueTail ].event = event;
	eventQueue[ queueTail ].key = key;
	queueTail = (uint8_t)( ( queueTail + 1U ) % EVENT_QUEUE_SIZE );
	queueCount++;
}

static bool dequeue_event( KEY_EVENT_t * event, uint8_t * key ) {
	if ( queueCount == 0U ) {
		return false;
	}

	*event = eventQueue[ queueHead ].event;
	*key = eventQueue[ queueHead ].key;
	queueHead = (uint8_t)( ( queueHead + 1U ) % EVENT_QUEUE_SIZE );
	queueCount--;
	return true;
}

/* Zwraca kod naciśniętego przycisku albo NO_KEY. */
static uint8_t read_pressed_key( void ) {
#ifdef BTN1_PORT
	if ( gpio_pin_READ( BTN1_PORT, BTN1_PIN ) == 0 ) {
		return KEY1;
	}
#endif
#ifdef BTN2_PORT
    if (gpio_pin_READ(BTN2_PORT, BTN2_PIN) == 0) {
        return KEY2;
    }
#endif
#ifdef BTN3_PORT
    if (gpio_pin_READ(BTN3_PORT, BTN3_PIN) == 0) {
        return KEY3;
    }
#endif
	return NO_KEY;
}

static bool duration_is_short( uint32_t duration ) {
	return ( duration >= SHORT_TIME_MIN ) && ( duration <= SHORT_TIME_MAX );
}

static void classify_non_click_press( uint32_t duration, uint8_t key ) {
	if ( duration < SHORT_TIME_MIN ) {
		/* Bardzo krótkie impulsy/zakłócenia ignorujemy. */
		return;
	}
	if ( duration <= SHORT_TIME_MAX ) {
		queue_event( SHORT_PRESS, key );
	} else if ( duration < MEDIUM_TIME_MAX ) {
		queue_event( MEDIUM_PRESS, key );
	} else {
		queue_event( LONG_PRESS, key );
	}
}

/* Obsługa zwolnienia przycisku po zakończeniu debounce. */
static void on_release( uint32_t now, uint8_t key ) {
	uint32_t duration = now - pressStart;

	if ( longEventSent ) {
		longEventSent = false;
		repeatCounter = 0U;
		return;
	}

	if ( clickState == CLICK_SECOND_DOWN ) {
		if ( duration_is_short( duration ) ) {
			queue_event( DOUBLE_PRESS, firstClickKey != NO_KEY ? firstClickKey : key );
			clickState = CLICK_IDLE;
			firstClickKey = NO_KEY;
		} else {
			/* Drugie naciśnięcie nie było krótkim kliknięciem.
			 Nie gubimy pierwszego oczekującego kliknięcia. */
			queue_event( SHORT_PRESS, firstClickKey );
			classify_non_click_press( duration, key );
			clickState = CLICK_IDLE;
			firstClickKey = NO_KEY;
		}
		return;
	}

	if ( duration_is_short( duration ) ) {
		/* Pojedynczego kliknięcia jeszcze nie zgłaszamy:
		 czekamy na ewentualne drugie kliknięcie. */
		clickState = CLICK_WAIT_SECOND;
		firstReleaseTime = now;
		firstClickKey = key;
	} else {
		/* Dłuższe przytrzymanie nie bierze udziału w double-click. */
		if ( clickState == CLICK_WAIT_SECOND ) {
			queue_event( SHORT_PRESS, firstClickKey );
			clickState = CLICK_IDLE;
			firstClickKey = NO_KEY;
		}
		classify_non_click_press( duration, key );
	}
}

static void process_button( void ) {
	uint32_t now = millis();
	uint8_t key = read_pressed_key();
	uint8_t rawPressed = ( key != NO_KEY ) ? KEY_PRESSED : KEY_RELEASED;

	/* Debounce: stan surowy musi pozostać niezmienny przez DEBOUNCE_TIME. */
	if ( rawPressed != rawCandidate ) {
		rawCandidate = rawPressed;
		candidateSince = now;
	}

	if ( ( rawCandidate != stablePressed ) && ( (uint32_t)( now - candidateSince ) >= DEBOUNCE_TIME ) ) {

		stablePressed = rawCandidate;

		if ( stablePressed == KEY_PRESSED ) {
			/* Po pierwszym zwolnieniu drugie naciśnięcie musi zacząć się
			 w oknie DOUBLE_PRESS_TIME. */
			if ( clickState == CLICK_WAIT_SECOND ) {
				if ( (uint32_t)( now - firstReleaseTime ) <= DOUBLE_PRESS_TIME ) {
					clickState = CLICK_SECOND_DOWN;
				} else {
					queue_event( SHORT_PRESS, firstClickKey );
					clickState = CLICK_IDLE;
					firstClickKey = NO_KEY;
				}
			}

			currentPressKey = key;
			pressStart = now;
			nextRepeatAt = now + MEDIUM_TIME_MAX;
			repeatCounter = 0U;
			longEventSent = false;
		} else {
			on_release( now, currentPressKey );
			currentPressKey = NO_KEY;
		}
	}

	/* Długie przytrzymanie i autorepeat. */
	if ( stablePressed == KEY_PRESSED ) {
		if ( (uint32_t)( now - pressStart ) >= MEDIUM_TIME_MAX ) {
			if ( !longEventSent ) {
				/* Długie drugie naciśnięcie nie może pozostawić automatu
				 double-click w stanie oczekiwania na zwolnienie. */
				if ( clickState == CLICK_SECOND_DOWN ) {
					queue_event( SHORT_PRESS, firstClickKey );
					clickState = CLICK_IDLE;
					firstClickKey = NO_KEY;
				}
				longEventSent = true;
				queue_event( LONG_PRESS, currentPressKey );
				nextRepeatAt = now + REPEAT_TIME;
				repeatCounter = 0U;
			} else if ( (int32_t)( now - nextRepeatAt ) >= 0 ) {
				nextRepeatAt += REPEAT_TIME;
				repeatCounter++;

				if ( repeatCounter >= VLONG_TIME_CNT ) {
					queue_event( VLONG_PRESS, currentPressKey );
					repeatCounter = 0U;
				} else {
					queue_event( KEY_REPEAT, currentPressKey );
				}
			}
		}
	}

	/* Upłynęło okno oczekiwania na drugie kliknięcie. */
	if ( ( clickState == CLICK_WAIT_SECOND ) && ( (uint32_t)( now - firstReleaseTime ) > DOUBLE_PRESS_TIME ) ) {
		queue_event( SHORT_PRESS, firstClickKey );
		clickState = CLICK_IDLE;
		firstClickKey = NO_KEY;
	}
}

void KEYBOARD_init( void ) {
	uint8_t keyNo = (uint8_t)( sizeof( KeysIN ) / sizeof( KeysIN[ 0 ] ) );

	for ( uint8_t i = 0U; i < keyNo; i++ ) {
		gpio_pin_cfg( KeysIN[ i ].port, KeysIN[ i ].pin, gpio_mode_in_PU );
	}

	keyboard.keyEvent = NO_EVENT;
	keyboard.keyCoded = NO_KEY;

	stablePressed = KEY_RELEASED;
	rawCandidate = KEY_RELEASED;
	candidateSince = millis();
	pressStart = 0U;
	nextRepeatAt = 0U;
	currentPressKey = NO_KEY;
	repeatCounter = 0U;
	longEventSent = false;

	clickState = CLICK_IDLE;
	firstReleaseTime = 0U;
	firstClickKey = NO_KEY;

	queueHead = 0U;
	queueTail = 0U;
	queueCount = 0U;
}

void SW_KEYBOARD_EVENT( KEYB_OUT_t * keyboard_ptr ) {
	KEY_EVENT_t event = NO_EVENT;
	uint8_t key = NO_KEY;

	if ( keyboard_ptr == NULL ) {
		return;
	}

	process_button();

	keyboard_ptr->keyEvent = NO_EVENT;
	keyboard_ptr->keyCoded = NO_KEY;

	if ( dequeue_event( &event, &key ) ) {
		keyboard.keyEvent = event;
		keyboard.keyCoded = key;
		*keyboard_ptr = keyboard;
	} else {
		keyboard.keyEvent = NO_EVENT;
	}
}

bool key_detect( void ) {
	if ( keyboard.keyEvent == SHORT_PRESS ) {
		keyboard.keyEvent = NO_EVENT;
		keyboard.keyCoded = NO_KEY;
		return true;
	}

	return false;
}

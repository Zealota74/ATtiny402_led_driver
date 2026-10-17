/*
 * keyboard_defs.h
 *
 * Konfiguracja i typy modułu obsługi klawiatury.
 */

#ifndef LIBS_SW_INPUT_KEYBOARD_KEYBOARD_DEFS_H_
#define LIBS_SW_INPUT_KEYBOARD_KEYBOARD_DEFS_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * Wszystkie czasy podane są w milisekundach.
 * Moduł powinien być wywoływany regularnie, np. co 10 ms.
 */
#define DEBOUNCE_TIME        20U

/* Klasyfikacja czasu przytrzymania. */
#define SHORT_TIME_MIN       30U
#define SHORT_TIME_MAX       200U
#define MEDIUM_TIME_MIN      200U
#define MEDIUM_TIME_MAX      700U

/* Maksymalny odstęp od zwolnienia pierwszego kliknięcia
 * do rozpoczęcia drugiego kliknięcia.
 */
#define DOUBLE_PRESS_TIME    150U

#define REPEAT_TIME          100U

/* Liczba okresów REPEAT_TIME do zgłoszenia VLONG_PRESS.
 * 20 oznacza ok. 2 s od rozpoczęcia powtarzania.
 */
#define VLONG_TIME_CNT       20U

#define NO_KEY               0U
#define KEY1                 1U
#define KEY2                 2U
#define KEY3                 3U

typedef enum {
    NO_EVENT = 0,
    SHORT_PRESS,
    MEDIUM_PRESS,
    LONG_PRESS,
    VLONG_PRESS,
    DOUBLE_PRESS,
    KEY_REPEAT
} KEY_EVENT_t;

typedef struct {
    KEY_EVENT_t keyEvent;
    uint8_t keyCoded;
} KEYB_OUT_t;

typedef struct {
    GPIO_TypeDef *port;
    T_GPIO_PIN pin;
} KEYB_IN_t;

#endif /* LIBS_SW_INPUT_KEYBOARD_KEYBOARD_DEFS_H_ */

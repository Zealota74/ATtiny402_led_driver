#ifndef BOOL_H_
#define BOOL_H_

#include <stdint.h>

/* 1. Definicja typu bool jako uint8_t (optymalne dla 8-bitowych AVR) */
typedef uint8_t bool;

/* 2. Definicja makr logicznych */
#define false 0
#define true  1

/* 3. Przydatne makra do operacji na flagach i bitach (często używane w AVR) */

// Zamienia dowolną wartość (np. wynik operacji bitowej) na czyste 0 lub 1
#define TO_BOOL(x)       ((x) ? true : false)

// Odwraca stan zmiennej bool (np. miganie diodą: stan = TOGGLE(stan);)
#define TOGGLE(x)        ((x) ^= 1)

// Zwraca tekstową reprezentację (przydatne do debugowania przez UART / LCD)
#define BOOL_TO_STR(x)   ((x) ? "TRUE" : "FALSE")

#endif /* BOOL_H_ */

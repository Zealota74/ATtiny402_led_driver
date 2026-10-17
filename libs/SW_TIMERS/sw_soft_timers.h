/*
 * sw_soft_timers.h
 * Przepisany dla ATtiny402 (Wielokana�owy programowy timer)
 */

#ifndef SW_SOFT_TIMERS_H_
#define SW_SOFT_TIMERS_H_

#include <stdint.h>

#define SYSTICK_MS		10

// Deklaracje zmiennych globalnych timer�w programowych
extern volatile uint16_t softTimer1, softTimer2, softTimer3, softTimer4, softTimer5;
extern volatile uint16_t debounceTimer, analyzeTimer;

static inline void sw_soft_timer_start( void ) {
    TCB0.CTRLA |= TCB_ENABLE_bm;
}
static inline void sw_soft_timer_stop( void ) {
    TCB0.CTRLA &= ~TCB_ENABLE_bm;
}

// Publiczne API modu�u
extern void sw_soft_timer_init(void);
extern uint32_t millis(void);

#endif /* SW_SOFT_TIMERS_H_ */

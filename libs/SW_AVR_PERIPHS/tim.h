/*
 * tim.h
 * Elastyczna konfiguracja od 1 do 3 kana��w PWM dla ATtiny402
 */

#ifndef TIM_H_
#define TIM_H_

#include <avr/io.h>
#include "tim_defs.h"
#include "gpio.h"


// --- WYB�R AKTYWNYCH KANA��W (1 = w��czony, 0 = wy��czony) ---
#define USE_PWM_CH1             1 // PA3 - pin 7
#define USE_PWM_CH2             0 // PA1 - pin 4
#define USE_PWM_CH3             0 // PA2 - pin 5


// Prototypy funkcji
extern void TIMER_pwm_init(void);
extern void TIMER_pwm_write(uint8_t chan, uint8_t val);
extern void TIMER_pwm_out_enable(uint8_t pwmOut);
extern void TIMER_pwm_out_disable(uint8_t pwmOut);
extern void TIMER_stop(void);
extern void TIMER_start(void);


#endif /* TIM_H_ */

/*
 * tim.h
 * TCA0 single-slope PWM, 8-bit, up to 3 outputs.
 * Outputs in use are selected in ATTINY402_led_driver.h (PWM_USED_MASK).
 */

#ifndef TIM_H_
#define TIM_H_

#include <avr/io.h>
#include "tim_defs.h"
#include "gpio.h"


extern void TIMER_pwm_init(void);
extern void TIMER_pwm_write(uint8_t chan, uint8_t val);
extern void TIMER_pwm_out_enable(uint8_t chan);
extern void TIMER_pwm_out_disable(uint8_t chan);
extern void TIMER_pwm_pins_cfg(T_GPIO_MODE mode);
extern void TIMER_stop(void);
extern void TIMER_start(void);


#endif /* TIM_H_ */

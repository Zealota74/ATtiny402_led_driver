/*
 * tim_defs.h
 * TCA0 waveform outputs of the ATtiny402 (SOIC-8)
 */

#ifndef TIM_DEFS_H_
#define TIM_DEFS_H_

#define PWM_WO0         0       // CMP0 -> PA3, or PA7 with PORTMUX (PWM_WO0_ALT = 1)
#define PWM_WO1         1       // CMP1 -> PA1
#define PWM_WO2         2       // CMP2 -> PA2
#define PWM_NONE        0xFF
#define PWM_CHANNELS    3

#define PWM_OUT_PORT    (&PORTA)

#define PWM_MASK(ch)    ( (ch) < PWM_CHANNELS ? (1 << ((ch) & 0x07)) : 0 )
#define PWM_WO0_PIN     ( PWM_WO0_ALT ? PA7 : PA3 )
#define PWM_PIN(ch)     ( (ch) == PWM_WO0 ? PWM_WO0_PIN : (ch) == PWM_WO1 ? PA1 : (ch) == PWM_WO2 ? PA2 : 0 )

#endif /* TIM_DEFS_H_ */

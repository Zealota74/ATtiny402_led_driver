/*
 * tim_defs.h
 * Przepisany dla ATtiny402 (TCA0)
 */

#ifndef TIM_DEFS_H_
#define TIM_DEFS_H_


// Definicje kana³ów zgodne z Twoim oryginalnym API
#define TIMER_CHA       0
#define TIMER_CHB       1
#define TIMER_CHC       2

#define TIMER_PWM1A_EN (1 << 0)
#define TIMER_PWM1B_EN (1 << 1)
#define TIMER_PWM1C_EN (1 << 2)

// Wybór portu dla wyjœæ PWM
#define PWM_OUT_PORT            (&PORTA)

// --- KONFIGURACJA PINÓW ---
#define PWM_OUT_PIN_OC1A             PA3   // Kana³ 1 (TCA0 WO0 -> CMP0)
#define PWM_OUT_PIN_OC1B             PA1   // Kana³ 2 (TCA0 WO1 -> CMP1)
#define PWM_OUT_PIN_OC1C             PA2   // Kana³ 3 (TCA0 WO2 -> CMP2)

// Maski do w³¹czania/wy³¹czania wyjœæ w locie
#define TIMER_CH1_EN            (1 << 0)
#define TIMER_CH2_EN            (1 << 1)
#define TIMER_CH3_EN            (1 << 2)

#endif /* TIM_DEFS_H_ */

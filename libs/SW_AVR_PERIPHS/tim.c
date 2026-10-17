/*
 * tim.c
 * Elastyczna obs�uga TCA0 dla 1, 2 lub 3 kana��w PWM
 */

#include <avr/io.h>
#include "gpio.h"
#include "tim.h"

void TIMER_pwm_init(void) {
    // 1. Konfiguracja pin�w dla aktywnych kana��w
#if USE_PWM_CH1
    gpio_pin_cfg(PWM_OUT_PORT, PWM_OUT_PIN_OC1A, gpio_mode_output_PP);
#endif

#if USE_PWM_CH2
    gpio_pin_cfg(PWM_OUT_PORT, PWM_OUT_PIN_OC1B, gpio_mode_output_PP);
#endif

#if USE_PWM_CH3
    gpio_pin_cfg(PWM_OUT_PORT, PWM_OUT_PIN_OC1C, gpio_mode_output_PP);
#endif

    // 2. Ustawienie trybu pracy: Single Slope PWM
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;

    // 3. Ustawienie okresu (TOP = 255 dla rozdzielczo�ci 8-bit)
    TCA0.SINGLE.PER = 255;

    // 4. Wst�pne wype�nienie (50%) dla aktywnych kana��w
#if USE_PWM_CH1
    TCA0.SINGLE.CMP0 = 127;
#endif

#if USE_PWM_CH2
    TCA0.SINGLE.CMP1 = 127;
#endif

#if USE_PWM_CH3
    TCA0.SINGLE.CMP2 = 127;
#endif

    // 5. Automatyczne uruchomienie wyj�� dla aktywnych kana��w
    uint8_t init_mask = 0;
#if USE_PWM_CH1
    init_mask |= TIMER_CH1_EN;
#endif
#if USE_PWM_CH2
    init_mask |= TIMER_CH2_EN;
#endif
#if USE_PWM_CH3
    init_mask |= TIMER_CH3_EN;
#endif
    TIMER_pwm_out_enable(init_mask);

    // 6. Uruchomienie timera TCA0 (podzia� zegara przez 16)
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
}
void TIMER_pwm_stop(void) {
	   TCA0.SINGLE.CTRLA &= TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
}

void TIMER_pwm_out_enable(uint8_t pwmOut) {
#if USE_PWM_CH1
    if ((pwmOut & TIMER_CH1_EN) == TIMER_CH1_EN) {
        TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP0EN_bm;
    }
#endif

#if USE_PWM_CH2
    if ((pwmOut & TIMER_CH2_EN) == TIMER_CH2_EN) {
        TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP1EN_bm;
    }
#endif

#if USE_PWM_CH3
    if ((pwmOut & TIMER_CH3_EN) == TIMER_CH3_EN) {
        TCA0.SINGLE.CTRLB |= TCA_SINGLE_CMP2EN_bm;
    }
#endif
}

void TIMER_pwm_out_disable(uint8_t pwmOut) {
#if USE_PWM_CH1
    if ((pwmOut & TIMER_CH1_EN) == TIMER_CH1_EN) {
        TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP0EN_bm;
    }
#endif

#if USE_PWM_CH2
    if ((pwmOut & TIMER_CH2_EN) == TIMER_CH2_EN) {
        TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP1EN_bm;
    }
#endif

#if USE_PWM_CH3
    if ((pwmOut & TIMER_CH3_EN) == TIMER_CH3_EN) {
        TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_CMP2EN_bm;
        // Opcjonalne zabezpieczenie stanu pinu po wy��czeniu PWM (np. dla LED_HI_POWER)
        gpio_pin_HI(PWM_OUT_PORT, PWM_OUT_PIN_OC1C);
    }
#endif
}

void TIMER_pwm_write(uint8_t chan, uint8_t val) {
    if (chan == TIMER_CHA) {
#if USE_PWM_CH1
        TCA0.SINGLE.CMP0 = val;
#endif
    } else if (chan == TIMER_CHB) {
#if USE_PWM_CH2
        TCA0.SINGLE.CMP1 = val;
#endif
    }
    // Mo�esz doda� obs�ug� trzeciego kana�u np. przez now� sta�� lub rozszerzenie logiki
    else if (chan == TIMER_CHC) { // Za��my, �e zdefiniujesz TIMER_CHC w pliku nag��wkowym
#if USE_PWM_CH3
        TCA0.SINGLE.CMP2 = val;
#endif
    }
}

// Globalne zatrzymanie ca�ego modu�u TCA0
void TIMER_stop(void) {
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}

// Globalne uruchomienie modu�u TCA0 (z zachowaniem lub ustawieniem preskalera)
void TIMER_start(void) {
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

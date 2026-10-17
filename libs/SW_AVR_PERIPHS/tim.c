/*
 * tim.c
 * TCA0 single-slope PWM, 8-bit, up to 3 outputs.
 */

#include <avr/io.h>
#include "gpio.h"
#include "tim.h"

// ------------------------ Board pin sanity checks -------------------------
#define PWM_USED(ch)        ( PWM_USED_MASK & PWM_MASK(ch) )
#define PWM_PINS_USED       ( ( PWM_USED(PWM_WO0) ? PWM_PIN(PWM_WO0) : 0 ) | \
                              ( PWM_USED(PWM_WO1) ? PWM_PIN(PWM_WO1) : 0 ) | \
                              ( PWM_USED(PWM_WO2) ? PWM_PIN(PWM_WO2) : 0 ) )

_Static_assert( LED_LO_PWM != LED_HI_PWM || LED_LO_PWM == PWM_NONE,
                "LED_LO_PWM and LED_HI_PWM use the same TCA0 output" );
_Static_assert( ( PWM_PINS_USED & KEY_PIN ) == 0,      "PWM output collides with KEY_PIN" );
_Static_assert( ( PWM_PINS_USED & LED_DIAG_PIN ) == 0, "PWM output collides with LED_DIAG_PIN" );
_Static_assert( KEY_PIN != LED_DIAG_PIN,               "KEY_PIN collides with LED_DIAG_PIN" );
_Static_assert( ( ( PWM_PINS_USED | KEY_PIN | LED_DIAG_PIN ) & PA0 ) == 0,
                "PA0 is reserved for UPDI" );
// --------------------------------------------------------------------------

static const T_GPIO_PIN pwmPins[ PWM_CHANNELS ] = { PWM_WO0_PIN, PA1, PA2 };

static inline uint8_t pwm_used( uint8_t chan ) {
    return ( chan < PWM_CHANNELS ) && ( PWM_USED_MASK & ( 1 << chan ) );
}

void TIMER_pwm_pins_cfg( T_GPIO_MODE mode ) {
    for ( uint8_t ch = 0; ch < PWM_CHANNELS; ch++ ) {
        if ( pwm_used( ch ) ) {
            gpio_pin_LO( PWM_OUT_PORT, pwmPins[ ch ] );     // level while the output is disabled
            gpio_pin_cfg( PWM_OUT_PORT, pwmPins[ ch ], mode );
        }
    }
}

void TIMER_pwm_init(void) {
    TIMER_pwm_pins_cfg( gpio_mode_output_PP );

#if PWM_WO0_ALT
    PORTMUX.CTRLC |= PORTMUX_TCA00_bm;
#endif

    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc;   // outputs stay disabled
    TCA0.SINGLE.PER   = 255;
    TCA0.SINGLE.CMP0  = 0;
    TCA0.SINGLE.CMP1  = 0;
    TCA0.SINGLE.CMP2  = 0;

    // F_CPU / 16 / 256: ~814 Hz at 3.33 MHz
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;
}

void TIMER_pwm_out_enable( uint8_t chan ) {
    if ( pwm_used( chan ) ) {
        TCA0.SINGLE.CTRLB |= ( TCA_SINGLE_CMP0EN_bm << chan );
    }
}

void TIMER_pwm_out_disable( uint8_t chan ) {
    if ( pwm_used( chan ) ) {
        TCA0.SINGLE.CTRLB &= ~( TCA_SINGLE_CMP0EN_bm << chan );
    }
}

void TIMER_pwm_write( uint8_t chan, uint8_t val ) {
    if ( !pwm_used( chan ) ) return;

    switch ( chan ) {
        case PWM_WO0: TCA0.SINGLE.CMP0 = val; break;
        case PWM_WO1: TCA0.SINGLE.CMP1 = val; break;
        case PWM_WO2: TCA0.SINGLE.CMP2 = val; break;
        default: break;
    }
}

void TIMER_stop(void) {
    TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
}

void TIMER_start(void) {
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

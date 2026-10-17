/*
 * rcc.c
 *
 *  Created on: 10 cze 2022
 *      Author: simon
 */


#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include "avr/interrupt.h"
#include <util/delay.h>

#include "gpio.h"
#include "adc.h"
#include "tim.h"

#include "rcc.h"

static uint8_t powerState = STATE_PCINT_DEBOUNCE;

void RCC_Clock_Init(void) {
    // 1. Odblokowanie rejestrów chronionych (Configuration Change Protection)
    CPU_CCP = CCP_IOREG_gc;

    #if F_CPU == 20000000L
        // 20 MHz: Wybieramy bazowy oscylator (20MHz), wy³¹czamy preskaler (podzia³ przez 1)
        CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc;
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLB = 0x00; // Preskaler wy³¹czony (PEN = 0)

    #elif F_CPU == 16000000L
        // 16 MHz: Przestrajamy bazowy generator na bieg 16MHz w rejestrze OSC20MCTRLA
        CLKCTRL.OSC20MCTRLA = CLKCTRL_RUNSTDBY_bm; // Wartoœæ 0x00 prze³¹cza na tryb 16MHz
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc; // Wybieramy generator g³ówny
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLB = 0x00; // Preskaler wy³¹czony

    #elif F_CPU == 8000000L
        // 8 MHz: Przestrajamy generator na bieg 16MHz + w³¹czamy sprzêtowy podzia³ przez 2 (16 / 2 = 8 MHz)
        CLKCTRL.OSC20MCTRLA = CLKCTRL_RUNSTDBY_bm;
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc;
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm; // Dzielnik 2X + Preskaler Enable

    #elif F_CPU == 3333333L
        // 3.33 MHz (Default): Generator na fabryczne 20MHz + sprzêtowy podzia³ przez 6 (20 / 6 = 3.33 MHz)
        CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc;
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_6X_gc | CLKCTRL_PEN_bm; // Dzielnik 6X + Preskaler Enable

    #elif F_CPU == 1000000L
        // 1 MHz: Przestrajamy generator na bieg 16MHz + sprzêtowy podzia³ przez 16 (16 / 16 = 1 MHz) [source: 1.4.1]
        CLKCTRL.OSC20MCTRLA = CLKCTRL_RUNSTDBY_bm;
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLA = CLKCTRL_CLKSEL_OSC20M_gc; [source: 1.4.1]
        CPU_CCP = CCP_IOREG_gc;
        CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_16X_gc | CLKCTRL_PEN_bm; // Dzielnik 16X + Preskaler Enable [source: 1.4.1]
    #endif
}

static inline void RCC_sleep_cpu(void) {
    __asm__ __volatile__ ( "sleep" "\n\t" :: );
}

void RCC_sleep_enable( uint8_t mode ) {
    gpio_pin_cfg( KEY_PORT,         KEY_PIN,            gpio_mode_in_floating );
    gpio_pin_cfg( LED_DIAG_PORT,    LED_DIAG_PIN,       gpio_mode_in_floating );
    gpio_pin_cfg( PWM_OUT_PORT,     PWM_OUT_PIN_OC1A,   gpio_mode_in_floating );
    gpio_pin_cfg( PWM_OUT_PORT,     PWM_OUT_PIN_OC1B,   gpio_mode_in_floating );
    gpio_pin_cfg( PWM_OUT_PORT,     PWM_OUT_PIN_OC1C,   gpio_mode_in_floating );

    powerState = STATE_PCINT_WAKE;
    ADC_disable();

    SLPCTRL.CTRLA = mode | SLPCTRL_SEN_bm;

    sei();
    RCC_sleep_cpu();                         // Idziemy spaæ w Power-Down
    SLPCTRL.CTRLA &= ~SLPCTRL_SEN_bm;
}

void RCC_sleep_disable(void) {
    SLPCTRL.CTRLA &= ~SLPCTRL_SEN_bm;
}



// Mapowanie funkcji WDT_enable / WDT_disable na sprzêtowy modu³ PIT w ATtiny402
void WDT_enable( uint8_t time_gc ) {
    RTC.PITINTCTRL = RTC_PI_bm;             // W³¹czenie przerwania Periodic Interrupt
    RTC.PITCTRLA = time_gc | RTC_PITEN_bm;  // Uruchomienie licznika PIT (np. na 64ms)
}

void WDT_disable(void) {
    RTC.PITCTRLA &= ~RTC_PITEN_bm;          // Wy³¹czenie licznika PIT
    RTC.PITINTCTRL &= ~RTC_PI_bm;           // Wy³¹czenie przerwaniowej flagi PIT
}

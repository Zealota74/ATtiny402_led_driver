/*
 * power_down.c
 * Power-Down sleep / wake-up by the button. Pins come from ATTINY402_led_driver.h.
 *
 * Sequence (POWER_DOWN_request, called from the main loop by an INPUT callback):
 *   stop tick -> status LED off -> wait for release -> Power-Down (KEY = LEVEL)
 *   -> wake on press -> debounce press and release (RTC, Standby)
 *   -> INPUT_reset() -> start tick -> status LED on
 * The tick is stopped during the whole sequence, so INPUT sees none of it.
 *
 * KEY_PIN (PA1) is not a fully asynchronous pin: from Power-Down it can only
 * wake the MCU with BOTHEDGES or LEVEL sensing. LEVEL is used.
 */

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "ATTINY402_led_driver.h"
#include "gpio.h"
#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_TIMERS/sw_soft_timers.h"
#include "SW_INPUT/input.h"
#include "power_down.h"

#define KEY_PINCTRL     ( *( &(KEY_PORT)->PIN0CTRL + __builtin_ctz(KEY_PIN) ) )

typedef enum {
    CPU_RUNNING,        // normal operation
    CPU_SLEEPING,       // LEVEL wake-up armed, MCU in Power-Down
    CPU_WAKE_DEBOUNCE   // woken up, waiting for the key to settle
} CPU_STATE_t;

static volatile CPU_STATE_t cpuState = CPU_RUNNING;
static volatile uint8_t     rtcDone;

// ---------------------------- Private functions -----------------------------
static inline uint8_t key_pressed(void) {
    return gpio_pin_READ( KEY_PORT, KEY_PIN ) == 0;    // key to GND: low = pressed
}

static void rtc_init(void) {
    while ( RTC.STATUS );
    RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;
    while ( RTC.STATUS );
    RTC.CTRLA  = RTC_RTCEN_bm | RTC_RUNSTDBY_bm;
}

// The only time base used while going to sleep / waking up.
// Waits in Standby, woken only by RTC CMP (interrupt enabled just here).
static void rtc_delay( uint16_t ticks ) {
    rtcDone = 0;

    while ( RTC.STATUS );
    RTC.CNT = 0;
    while ( RTC.STATUS );
    RTC.CMP      = ticks;
    RTC.INTFLAGS = RTC_CMP_bm;
    RTC.INTCTRL  = RTC_CMP_bm;

    set_sleep_mode( SLEEP_MODE_STANDBY );
    cli();
    while ( !rtcDone ) {
        sleep_enable();
        sei();
        sleep_cpu();        // the instruction after sei() runs before the ISR: no race
        sleep_disable();
        cli();
    }
    sei();
}

// Waits for the key release (polling) + release debounce
static void wait_key_released(void) {
    while ( key_pressed() ) {
        rtc_delay( RELEASE_POLL_TICKS );
    }
    rtc_delay( RELEASE_DEBOUNCE_TICKS );
}

// Arming the wake-up is the LAST operation before sleep_cpu()
static void power_down(void) {
    set_sleep_mode( SLEEP_MODE_PWR_DOWN );

    cli();
    if ( key_pressed() ) {          // pressed right now: do not sleep,
        sei();                      // the rest is handled like a wake-up
        return;
    }
    cpuState         = CPU_SLEEPING;
    (KEY_PORT)->INTFLAGS = KEY_PIN;
    KEY_PINCTRL      = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;
    sleep_enable();
    sei();
    sleep_cpu();                    // sleeps here, returns after ISR(PORTA_PORT_vect)
    sleep_disable();
}

// After wake-up the key is still pressed and bouncing.
static void wake_settle(void) {
    rtc_delay( WAKE_DEBOUNCE_TICKS );   // debounce the press that woke the MCU
    wait_key_released();                // release + release debounce

    cli();
    INPUT_reset();                      // INPUT: key released, idle
    cpuState = CPU_RUNNING;
    sei();
    sw_soft_timer_start();              // tick restarts only in a stable state

    sw_led_on();                        // ready for normal operation
}

// ---------------------------- Public functions ------------------------------
void POWER_DOWN_init(void) {
    rtc_init();
    INPUT_EVENTS_cb_reg( EVENT_KEY1_VLONG_PRESS, POWER_DOWN_request );
    // to sleep after a short click instead: EVENT_KEY1_SHORT_PRESS
}

// VLONG_PRESS is reported while the key is still PRESSED, so wait for the
// release first (LEVEL may only be armed with the pin high).
// TODO (integration): switch PWM/ADC off before sleep and restore them after.
void POWER_DOWN_request(void) {
    sw_soft_timer_stop();               // INPUT must not see the rest of the sequence
    sw_led_off();                       // threshold reached / going to sleep

    wait_key_released();

    power_down();                       // returns after wake-up by the key
    wake_settle();
}

// -------------------------------- Interrupts --------------------------------
// Wake-up only: disables its own interrupt (LEVEL would retrigger), keeps pull-up
ISR( PORTA_PORT_vect ) {
    if ( ( (KEY_PORT)->INTFLAGS & KEY_PIN ) && cpuState == CPU_SLEEPING ) {
        KEY_PINCTRL = PORT_PULLUPEN_bm | PORT_ISC_INTDISABLE_gc;
        cpuState    = CPU_WAKE_DEBOUNCE;
    }
    (KEY_PORT)->INTFLAGS = KEY_PIN;
}

ISR( RTC_CNT_vect ) {
    RTC.INTFLAGS = RTC_CMP_bm;
    RTC.INTCTRL  = 0;
    rtcDone = 1;
}

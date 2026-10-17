/*
 * sw_ledlight_manage.c
 * Przepisany dla ATtiny402 z zachowaniem sprz�towego debouncingu oszcz�dzaj�cego energi�
 */

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <bool.h>

#include "rcc.h"

#include "gpio.h"
#include "tim.h"
#include "adc.h"

#include "SW_BOARD/sw_led_blink_debug.h"
#include "SW_INPUT/input.h"

#include "sw_ledlight_manage.h"

GPIO_EXTI_t LEDLIGHT_trigger;


// -------------------------- Private data -------------------------------
static uint8_t measureStatus = adc_status_init;
static bool lockMode         = LOCK_MODE_OFF;
static uint8_t brightIndex  = 2;
static uint8_t ledMode       = LED_MODE_LOW;
static uint8_t BrightGamma[ BRIGHT_LEVELS ] = { 0, 8, 38, 68, 130, 255 };

uint8_t EEMEM eeprom_valid     = EEPROM_VALID_VAL;
uint8_t EEMEM eeprom_GAMMA_LED = EEPROM_GAMMA_INIT;
uint8_t EEMEM eeprom_MODE      = EEPROM_LEDMODE_INIT;
// --------------------------------------------------------------------------

// ----------------------------- Private functions ---------------------------
static void LEDLIGHT_bright_set( uint8_t ledNum, uint8_t brightLoc ) {
    brightLoc = BrightGamma[ brightLoc ];
    if ( ledNum == LED_HIGH_POWER )
        TIMER_pwm_write( TIMER_CHB, brightLoc );
    else
    if ( ledNum == LED_LOW_POWER )
        TIMER_pwm_write( TIMER_CHA, brightLoc );
}

void LEDLIGHT_more_light(void) {
    if ( ++brightIndex == BRIGHT_LEVEL_MAX + 1 ) brightIndex = BRIGHT_LEVEL_MIN;
    LEDLIGHT_bright_set( LED_LOW_POWER,  brightIndex );
    LEDLIGHT_bright_set( LED_HIGH_POWER, brightIndex );
}

void LEDLIGHT_power_down(void) {
    TIMER_pwm_out_disable( TIMER_PWM1A_EN );
    TIMER_pwm_out_disable( TIMER_PWM1B_EN );
    TIMER_stop();

    volatile uint8_t *key_ctrl = KEY_PORT.PIN0CTRL + __builtin_ctz(KEY_PIN);
    *key_ctrl = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc;

    gpio_pin_IMPULSES( LED_DIAG_PORT, LED_DIAG_PIN, 1 );
    sw_led_off();
}

void LEDLIGHT_mode_set( uint8_t mode ) {
    switch ( mode ) {
        case LED_MODE_ALL_OFF:
            TIMER_pwm_out_disable( TIMER_PWM1A_EN );
            TIMER_pwm_out_disable( TIMER_PWM1B_EN );
            break;
        case LED_MODE_LOW:
            TIMER_pwm_out_enable ( TIMER_PWM1A_EN );
            TIMER_pwm_out_disable( TIMER_PWM1B_EN );
            break;
        case LED_MODE_HIGH:
            TIMER_pwm_out_disable( TIMER_PWM1A_EN );
            TIMER_pwm_out_enable ( TIMER_PWM1B_EN );
            break;
        case LED_MODE_LOW_HIGH:
            TIMER_pwm_out_enable ( TIMER_PWM1A_EN );
            TIMER_pwm_out_enable ( TIMER_PWM1B_EN );
            break;
        default:
            break;
    }
}

static void LEDLIGHT_mode_switch( void ) {
    if ( lockMode == LOCK_MODE_ON ) return;
    switch ( ledMode ) {
        case LED_MODE_LOW:      ledMode = LED_MODE_HIGH;     break;
        case LED_MODE_HIGH:     ledMode = LED_MODE_LOW_HIGH; break;
        case LED_MODE_LOW_HIGH: ledMode = LED_MODE_LOW;      break;
        default: break;
    }
    LEDLIGHT_mode_set( ledMode );
}

void LEDLIGHT_mode_lock( bool lock ) {
    lockMode = lock;
}

static void EEPROM_init(void) {
    if( eeprom_read_byte( &eeprom_valid ) != EEPROM_VALID_VAL ) {
        eeprom_update_byte( &eeprom_valid,     EEPROM_VALID_VAL );
        eeprom_update_byte( &eeprom_GAMMA_LED, EEPROM_GAMMA_INIT );
        eeprom_update_byte( &eeprom_MODE,      EEPROM_LEDMODE_INIT );

        brightIndex = 0;
        ledMode     = LED_MODE_LOW_HIGH;
    } else {
        ledMode     = eeprom_read_byte( &eeprom_MODE );
        brightIndex = eeprom_read_byte( &eeprom_GAMMA_LED );
    }
    LEDLIGHT_mode_set( ledMode );
    TCA0.SINGLE.CMP0 = BrightGamma[ brightIndex ];
    TCA0.SINGLE.CMP1 = BrightGamma[ brightIndex ];
}

// ---------------------------- Public functions ----------------------------
void EEPROM_update(void) {
    eeprom_update_byte( &eeprom_GAMMA_LED, brightIndex );
    eeprom_update_byte( &eeprom_MODE,      ledMode );
}

static void LEDLIGHT_pin_init(void) {
    gpio_pin_cfg( PWM_OUT_PORT, PWM_OUT_PIN_OC1A, gpio_mode_output_PP );
    gpio_pin_cfg( PWM_OUT_PORT, PWM_OUT_PIN_OC1B, gpio_mode_output_PP );
    gpio_pin_cfg( PWM_OUT_PORT, PWM_OUT_PIN_OC1C, gpio_mode_output_PP );
//    gpio_pin_HI( PWM_OUT_PORT, PWM_OUT_PIN_OC1A );
//    gpio_pin_HI( PWM_OUT_PORT, PWM_OUT_PIN_OC1B );
//    gpio_pin_HI( PWM_OUT_PORT, PWM_OUT_PIN_OC1C );
}

void LEDLIGHT_init(void) {
	LEDLIGHT_pin_init();

    TIMER_pwm_init();
    EEPROM_init();
    INPUT_init();

	INPUT_EVENTS_cb_reg( EVENT_KEY1_SHORT_PRESS,	LEDLIGHT_mode_switch );
	INPUT_EVENTS_cb_reg( EVENT_KEY1_MEDIUM_PRESS,	LEDLIGHT_more_light );
	INPUT_EVENTS_cb_reg( EVENT_KEY1_VLONG_PRESS,	LEDLIGHT_power_down );
	INPUT_EVENTS_cb_reg( EVENT_KEY1_DOUBLE_PRESS,	sw_led_blink );
	INPUT_EVENTS_cb_reg( EVENT_KEY1_REPEAT_PRESS,	sw_led_blink );
}

void LEDLIGHT_wakeup(void) {
    measureStatus = adc_status_init;
    ADC_enable();

    ledMode = eeprom_read_byte( &eeprom_MODE );
    LEDLIGHT_mode_set( ledMode );
    TIMER_start();

    gpio_pin_cfg( KEY_PORT,          KEY_PIN,           gpio_mode_in_PU );
    gpio_pin_cfg( LED_DIAG_PORT,     LED_DIAG_PIN,      gpio_mode_output_PP );
    gpio_pin_cfg( PWM_OUT_PORT,      PWM_OUT_PIN_OC1A,  gpio_mode_output_PP );
    gpio_pin_cfg( PWM_OUT_PORT,      PWM_OUT_PIN_OC1B,  gpio_mode_output_PP );

    _delay_ms(1000);
    if ( gpio_pin_READ( KEY_PORT, KEY_PIN) == 0 ) {
        brightIndex = BRIGHT_LEVEL_MAX;
        LEDLIGHT_bright_set( LED_LOW_POWER,  brightIndex );
        LEDLIGHT_bright_set( LED_HIGH_POWER, brightIndex );
    }
    _delay_ms(1000);

    sw_led_on();
}

static int8_t counter = 0;
void LEDLIGHT_check_battery(void) {
    uint16_t adcOut = ADC_mean_get();

    if ( (adcOut < ADC_THRESHOLD_MIN) && (measureStatus == adc_status_under || measureStatus == adc_status_init) ) {
        if (counter++ == 10) {
            counter = 0;
            measureStatus = adc_status_over;
            DEBUG_blink_start( 20, 25 );
            LEDLIGHT_mode_set( LED_MODE_ALL_OFF );
            LEDLIGHT_mode_lock( LOCK_MODE_ON );
        }
    }
    else if ( (adcOut > ADC_THRESHOLD_MAX) && (measureStatus == adc_status_over || measureStatus == adc_status_init) ) {
        if (counter-- == -10)  {
            measureStatus = adc_status_under;
            counter = 0;
            DEBUG_blink_stop();
            LEDLIGHT_mode_lock( LOCK_MODE_OFF );
            LEDLIGHT_mode_set( ledMode );
        }
    }
}

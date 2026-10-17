/*
 * sw_ledlight_manage.h
 * Przepisany dla ATtiny402 (Zarz¹dzanie trybami latarki)
 */

#ifndef SW_LEDLIGHT_MANAGE_H_
#define SW_LEDLIGHT_MANAGE_H_


#define BRIGHT_LEVELS       6
#define BRIGHT_LEVEL_MAX    5
#define BRIGHT_LEVEL_MIN    0

#define EEPROM_VALID_VAL    0x42
#define EEPROM_GAMMA_INIT   5
#define EEPROM_LEDMODE_INIT LED_MODE_LOW

#define LOCK_MODE_ON        true
#define LOCK_MODE_OFF       false

// Mapowanie logicznych diod na kana³y sprzêtowe Timera A (TCA0) z modu³u tim.h
#define LED_HIGH_POWER      LED_HI_PWM // ATTINY402_led_driver.h
#define LED_LOW_POWER       LED_LO_PWM // ATTINY402_led_driver.h

enum {
    LED_BRIGHT_OFF = 0,
    LED_BRIGHT_FULLPOWER = 255
};

enum {
    LED_MODE_LOW       = 1,
    LED_MODE_HIGH      = 2,
    LED_MODE_LOW_HIGH  = 3,
    LED_MODE_ALL_OFF   = 4,
    LED_MODE_SWITCH    = 5,
    LED_MODE_NO_SWITCH = 6
};

// Sta³e czasu dla nowego Watchdoga w ATtiny402 (WDT.CTRLA)
#define WDT_PERIOD_64MS    WDT_PERIOD_64CLK_gc
#define WDT_PERIOD_8S      WDT_PERIOD_8KCLK_gc

// Publiczne API modu³u zgodne z oryginalnym interfejsem
extern void LEDLIGHT_init(void);
extern void EEPROM_update(void);
extern void LEDLIGHT_mode_lock( bool lock );
extern void LEDLIGHT_check_battery(void);
extern void LEDLIGHT_mode_set( uint8_t mode );
extern void LEDLIGHT_power_down(void);
extern void LEDLIGHT_wakeup(void);

extern void RCC_sleep_enable( uint8_t mode );
extern void RCC_sleep_disable(void);
extern void WDT_enable( uint8_t time_gc );
extern void WDT_disable(void);

extern void LEDLIGHT_more_light(void);

#endif /* SW_LEDLIGHT_MANAGE_H_ */

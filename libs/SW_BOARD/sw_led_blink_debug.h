/*
 * sw_led_blink.h
 *
 *  Created on: 05.07.2017
 *      Author: simon
 */

#ifndef SW_LED_BLINK_H_
#define SW_LED_BLINK_H_


//enum { _led_blinking, _led_not_blinking };

extern void sw_led_debug_init(void);
extern void sw_led_on(void);
extern void sw_led_off(void);
extern void sw_led_blink(void);

extern int8_t DEBUNG_blink_get_status(void);
extern void   DEBUG_blink_start( uint8_t repeat, uint16_t delay );
extern void	  DEBUG_blink_stop(void);
extern void   DEBUG_BLINK_EVENT(void);

#endif /* SW_LED_BLINK_H_ */

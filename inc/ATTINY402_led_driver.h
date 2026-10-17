/*
 * ATTINY402_led_driver.h
 * Board configuration: the only place where pins are assigned.
 *
 *  Created on: 18.07.2019
 *      Author: simon
 */

#ifndef ATTINY402_LED_DRIVER_H_
#define ATTINY402_LED_DRIVER_H_

#include "tim_defs.h"

//#define DEBUG


#if DEBUG
#define DEBUG_PORT0			&PORTA
#define DEBUG_PIN0			PA6
#endif

#define LED_DIAG_PORT		&PORTA
#define LED_DIAG_PIN		PA7

#define KEY_PORT			&PORTA
#define KEY_PIN				PA1

/*
 * LED outputs, driven by TCA0 PWM. Pick one of:
 *   PWM_WO0  - PA3 (pin 7), or PA7 (pin 3) when PWM_WO0_ALT = 1
 *   PWM_WO1  - PA1 (pin 4)
 *   PWM_WO2  - PA2 (pin 5)
 *   PWM_NONE - channel not fitted
 * Pin conflicts are rejected at compile time (see tim.c).
 */
#define PWM_WO0_ALT			0
#define LED_LO_PWM			PWM_WO0
#define LED_HI_PWM			PWM_WO2

#define PWM_USED_MASK		( PWM_MASK(LED_LO_PWM) | PWM_MASK(LED_HI_PWM) )


#endif

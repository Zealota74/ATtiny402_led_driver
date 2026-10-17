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
#ifndef LED_DIAG_PIN
#define LED_DIAG_PIN		PA7
#endif

#define KEY_PORT			&PORTA
#ifndef KEY_PIN
#define KEY_PIN				PA1
#endif

/*
 * LED outputs, driven by TCA0 PWM. Pick one of:
 *   PWM_WO0  - PA3 (pin 7), or PA7 (pin 3) when PWM_WO0_ALT = 1
 *   PWM_WO1  - PA1 (pin 4)
 *   PWM_WO2  - PA2 (pin 5)
 *   PWM_NONE - channel not fitted
 * Pin conflicts are rejected at compile time (see tim.c).
 * Each option can also be set from the project symbols, e.g. -DPWM_WO0_ALT=1
 * (Eclipse: Properties > C/C++ Build > Settings > AVR Compiler > Symbols).
 */
#ifndef PWM_WO0_ALT
#define PWM_WO0_ALT			0
#endif
#ifndef LED_LO_PWM
#define LED_LO_PWM			PWM_WO0
#endif
#ifndef LED_HI_PWM
#define LED_HI_PWM			PWM_WO2
#endif

#define PWM_USED_MASK		( PWM_MASK(LED_LO_PWM) | PWM_MASK(LED_HI_PWM) )


#endif

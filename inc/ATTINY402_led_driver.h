/*
 * STM32F070F6Px_devboard.h
 *
 *  Created on: 18.07.2019
 *      Author: simon
 */

#ifndef ATTINY402_LED_DRIVER_H_
#define ATTINY402_LED_DRIVER_H_


//#define DEBUG


#if DEBUG
#define DEBUG_PORT0			&PORTA
#define DEBUG_PIN0			PA4
#endif

#define BAT_SENS_PORT		&PORTA
#define BAT_SENS_PIN		PA4

#define LED_DIAG_PORT		&PORTA
#define LED_DIAG_PIN		PA7

#define KEY_PORT			&PORTA
#define KEY_PIN				PA1

#define LED_HI_POWER_PORT	&PORTA
#define LED_LO_POWER_PORT	&PORTA
#define LED_HI_POWER_PIN	PA3		// ~OC1A
#define LED_LO_POWER_PIN	PA6		// ~OC1B


#endif

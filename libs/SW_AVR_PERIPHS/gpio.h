/*
 * gpio.h
 *
 *  Created on: 06.07.2017
 *      Author: simon
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "gpio_defs.h"

// Deklaracje funkcji GPIO
extern void gpio_pin_cfg( GPIO_TypeDef * const port, T_GPIO_PIN pin, T_GPIO_MODE mode);
extern void gpio_pin_LO( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void gpio_pin_HI( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void gpio_pin_XOR( GPIO_TypeDef * const port, T_GPIO_PIN pin );
extern void RCC_gpio_init( void );

extern void gpio_pin_IMPULSES( GPIO_TypeDef * const port, T_GPIO_PIN pin, uint8_t num );

// Odpowiednik odczytu rejestru IDR z STM32 za pomoc¹ rejestru IN z AVR
static inline int8_t gpio_pin_READ( GPIO_TypeDef * const port, T_GPIO_PIN pin ) {
    if ( port->IN & pin )	return 1;
    else					return 0;
}

// Funkcja konfiguracji przerwania zewnêtrznego (Odpowiednik z STM32)
extern void GPIO_Configure_EXTI( const GPIO_EXTI_t * extiPIN );

#endif /* GPIO_H_ */

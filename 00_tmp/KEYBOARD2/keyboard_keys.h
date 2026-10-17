/*
 * keyboard_keys.h
 *
 *  Created on: 24 sie 2021
 *      Author: simon
 */

#ifndef SRC_LIBS_KEYBOARD_KEYS_H_
#define SRC_LIBS_KEYBOARD_KEYS_H_

#define KEYS_SIMPLE
//#define KEYS_MULTI
//#define KEYS_MATRIX


#ifdef 	KEYS_SIMPLE
//#	define NUM_KEYS				3
#	define BTN1_PORT			KEY_PORT
#	define BTN1_PIN				KEY_PIN
//#	define BTN2_PORT			&PORTB
//#	define BTN2_PIN				 PA11
//#	define BTN3_PORT			&PORTB
//#	define BTN3_PIN				 PA12


/****************************** Few keys manage *****************************/
static inline uint8_t sw_get_keys_simple( void ) {
#ifdef BTN1_PORT
	if ( gpio_pin_READ( BTN1_PORT, BTN1_PIN ) == 0 )
		return 1;									// Key 1 pressed
#endif
#ifdef BTN2_PORT
	else
	if ( gpio_pin_READ( BTN2_PORT, BTN2_PIN ) == 0 )
		return 2;
#endif
#ifdef BTN3_PORT
	else
	if ( lgpio_pin_READ( BTN3_PORT, BTN3_PIN ) == 0 )
		return 3;
#endif
	return 0;
}
/*****************************************************************************************/
#endif

#ifdef KEY_MULTI
#	define NUM_KEYS	10

#endif


#ifdef KEY_MATRIX
//TO DO
#endif

#endif /* SRC_LIBS_KEYBOARD_KEYS_H_ */

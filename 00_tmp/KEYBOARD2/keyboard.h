/*
 * keyboard.h
 *
 *  Created on: 12 sie 2021
 *      Author: simon
 */

#ifndef SRC_LIBS_KEYBOARD_H_
#define SRC_LIBS_KEYBOARD_H_

#include "keyboard_keys.h"
#include "keyboard_defs.h"

#define TEST_KEYBOARD	0

// ----------------- public declarations --------------------
extern void KEYBOARD_init(void);
extern void SW_KEYBOARD_EVENT( KEYB_OUT_t * keyboard_ptr );
extern bool key_detect( void );


#endif /* SRC_LIBS_KEYBOARD_H_ */

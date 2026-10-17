/*
 * sw_mcu_conf.h
 *
 *  Created on: 15.12.2017
 *      Author: simon
 */

#ifndef LIBS_SW_MCU_CONF_H_
#define LIBS_SW_MCU_CONF_H_


#include <avr/io.h>

#define INLINE inline 	__attribute__((always_inline))
#define INTERRUPT 		__attribute__((interrupt))

#define OPTIMISE(x)		__attribute__((optimize(x)))


//#ifndef bool
//#include <stdbool.h>
//#endif
#include "bool.h"

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))
#define CLEAR_REG(REG)        ((REG) = (0x0))
#define WRITE_REG(REG, VAL)   ((REG) = (VAL))
#define READ_REG(REG)         ((REG))
#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))



#endif /* LIBS_SW_MCU_CONF_H_ */

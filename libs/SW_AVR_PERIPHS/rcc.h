/*
 * rcc.h
 *
 *  Created on: 10 cze 2022
 *      Author: simon
 */

#ifndef LIBS_SW_AVR_PERIPHS_RCC_H_
#define LIBS_SW_AVR_PERIPHS_RCC_H_

/*
 * Trik naprawiaj¹cy parser Eclipse graficznego edytora.
 * Jeœli u³omny plugin wymusza w edytorze b³êdne F_CPU (np. 1MHz),
 * ten zapis nadpisze pamiêæ podrêczn¹ edytora wartoœci¹, któr¹ faktycznie dosta³ kompilator.
 */
#ifdef __CDT_PARSER__
    #undef F_CPU
    #define F_CPU 3333333L  // Wpisz tutaj czêstotliwoœæ, któr¹ aktualnie testujesz na poligonie
#endif

extern void RCC_Clock_Init(void);


#endif /* LIBS_SW_AVR_PERIPHS_RCC_H_ */

/*
 * rcc.h
 *
 *  Created on: 10 cze 2022
 *      Author: simon
 */

#ifndef LIBS_SW_AVR_PERIPHS_RCC_H_
#define LIBS_SW_AVR_PERIPHS_RCC_H_

enum {
	STATE_PCINT_INIT, STATE_PCINT_DEBOUNCE, STATE_PCINT_WAKE, STATE_PCINT_WDT
};

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

extern void RCC_sleep_enable( uint8_t mode );

extern void WDT_enable( uint8_t time );
extern void WDT_init( void );
extern void enter_sleep(void);

#endif /* LIBS_SW_AVR_PERIPHS_RCC_H_ */

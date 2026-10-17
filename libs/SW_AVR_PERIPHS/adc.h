/*
 * adc.h
 * Przepisany dla ATtiny402
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

#include "gpio.h"
#include "adc_defs.h"

/*
 * Skorygowany kalkulator dla VREF = 2.50V (zamiast starego 2.56V):
 * Dzielnik rezystorowy: 4.2V -> 68k | 100k -> 2.50V na wejœciu pinu
 * 4.2V -> 1023 ADC - Maksymalne napiêcie
 * 3.6V -> 2.14V -> 877 ADC
 * 3.4V -> 2.02V -> 828 ADC
 * 3.3V -> 1.96V -> 804 ADC
 */
#define ADC_THRESHOLD_MAX   804
#define ADC_THRESHOLD_MIN   780

typedef enum {
    ADC_mean_ready,
    ADC_mean_NOT_ready
} ADC_MEAN_t;

enum {
	adc_status_under, adc_status_init, adc_status_over
};


// Publiczne API zgodne z Twoim dotychczasowym modu³em
extern void ADC_init(void);
extern void ADC_enable(void);
extern void ADC_disable(void);
extern void ADC_start_conv(void);
extern uint16_t ADC_mean_get(void);

extern void adc_event_cb_reg( void (*callback)( void ) );
extern void ADC_EVENT(void);

#endif /* ADC_H_ */

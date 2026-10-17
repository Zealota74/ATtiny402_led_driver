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
 * Supply voltage (VDD = battery) is measured without any external divider:
 * ADC reference = VDD, ADC input = internal 1.1 V reference.
 * RES = 4 * 1024 * 1.1 V / VDD  (4 accumulated samples)
 * VDD [mV] = ADC_VDD_CALC_CONST / RES
 */
#define ADC_INTREF_MV           1100UL
#define ADC_VDD_CALC_CONST      ( ADC_INTREF_MV * 1024UL * 4UL )

#define VDD_THRESHOLD_MIN_MV    3200    // below: LEDs off and locked
#define VDD_THRESHOLD_MAX_MV    3300    // above: LEDs back on

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
extern uint16_t ADC_vdd_mv_get(void);

extern void adc_event_cb_reg( void (*callback)( void ) );
extern void ADC_EVENT(void);

#endif /* ADC_H_ */

/*
 * adc_defs.h
 * Przepisany dla ATtiny402 (ADC0 & VREF)
 */

#ifndef ADC_DEFS_H_
#define ADC_DEFS_H_


// Definicje napiêæ odniesienia mapowane na rejestr VREF.CTRLA
#define ADC_VREF_VCC         0 // VCC jako napiêcie odniesienia
#define ADC_VREF_2_50V_INT   VREF_ADC0REFSEL_2V5_gc // Wewnêtrzne stabilne 2.50V (odpowiednik 2.56V)

// Definicje kana³ów wejœciowych mapowane na rejestr ADC0.MUXPOS
#define ADC_MUX_ADC3_PA3     ADC_MUXPOS_AIN3_gc     // Pin PA3 jako wejœcie analogowe AIN3
#define ADC_MUX_TEMP         ADC_MUXPOS_TEMPSENSE_gc // Wbudowany czujnik temperatury

// Definicje preskalera mapowane na rejestr ADC0.CTRLC
#define ADC_PSC_DIV_4        ADC_PRESC_DIV4_gc
#define ADC_PSC_DIV_16       ADC_PRESC_DIV16_gc
#define ADC_PSC_DIV_64       ADC_PRESC_DIV64_gc     // Dla 16/20 MHz daje idealn¹ czêstotliwoœæ ADC

#endif /* ADC_DEFS_H_ */

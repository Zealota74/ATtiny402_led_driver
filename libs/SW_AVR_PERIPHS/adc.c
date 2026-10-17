/*
 * adc.c
 * Przepisany dla ATtiny402
 */


#include <avr/interrupt.h>

#include "rcc.h"
#include "adc.h"

// Zmienne statyczne modu³u
static ADC_MEAN_t adcEndConvFlag = ADC_mean_NOT_ready;
static void (*adc_end_conv_event_cb)( void ) = 0;
static uint16_t adcRead = 0;

// Szybkie funkcje statyczne inline mapowane na now¹ strukturê ADC0 i VREF
static inline void ADC_vref_selec( uint8_t vRef ) {
    VREF.CTRLA = vRef;
}

static inline void ADC_single_in_set( uint8_t adcIn ) {
    ADC0.MUXPOS = adcIn;
}

static inline void ADC_prescaller_set( uint8_t psc ) {
    // W serii 0 preskaler i rozdzielczoœæ (10-bit) konfiguruje siê w CTRLC
    ADC0.CTRLC = psc | ADC_SAMPLEN_1_bm;
}

static inline void ADC_isr_enable(void) {
    ADC0.INTCTRL = ADC_RESRDY_bm; // W³¹czenie przerwania od gotowoœci wyniku (Result Ready)
}

/*********************************************************************************/
void ADC_enable(void) {
    ADC0.CTRLA |= ADC_ENABLE_bm;
}

void ADC_disable(void) {
    ADC0.CTRLA &= ~ADC_ENABLE_bm;
}

void ADC_start_conv( void ) {
    ADC0.COMMAND = ADC_STCONV_bm; // Atomowy start konwersji
}

uint16_t ADC_mean_get(void) {
    return adcRead;
}

/*********************************************************************************/
// Nowy, uproszczony wektor przerwania dla serii 0 (Zastêpuje stary ADC_vect)
ISR( ADC0_RESRDY_vect ) {
    // Pobieramy gotowy, automatycznie uœredniony sprzêtowo wynik z rejestru RES
    adcRead = ADC0.RES;

    // KRYTYCZNE: Flagi przerwañ w nowym AVR czyœci siê wpisuj¹c w nie '1'
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // Ustawiamy flagê gotowoœci dla maszyny zdarzeñ
    adcEndConvFlag = ADC_mean_ready;
}

/*********************************************************************************/
void ADC_init(void) {
    // 1. Wy³¹czenie bufora cyfrowego na pinie pomiarowym przez modu³ GPIO
    gpio_pin_cfg( BAT_SENS_PORT, BAT_SENS_PIN, gpio_mode_analog );

    // 2. Konfiguracja napiêcia referencyjnego na 2.50V
    ADC_vref_selec( ADC_VREF_2_50V_INT );

    // 3. Konfiguracja wejœcia analogowego
    ADC_single_in_set( ADC_MUX_ADC3_PA3 );

    // 4. AUTOMATYCZNY PRESKALER ADC (Dba o to, by taktowanie ADC wynosi³o ok. 125 kHz)
    #if F_CPU >= 16000000L
        ADC_prescaller_set( ADC_PRESC_DIV128_gc ); // 16-20 MHz / 128 = 125-156 kHz
    #elif F_CPU == 8000000L
        ADC_prescaller_set( ADC_PRESC_DIV64_gc );  // 8 MHz / 64 = 125 kHz
    #elif F_CPU == 3333333L
        ADC_prescaller_set( ADC_PRESC_DIV32_gc );  // 3.33 MHz / 32 = 104 kHz
    #elif F_CPU == 1000000L
        ADC_prescaller_set( ADC_PRESC_DIV8_gc );   // 1 MHz / 8 = 125 kHz
	#else
		#error "Nieobs³ugiwana czêstotliwoœæ F_CPU w sw_soft_timers.c!"
    #endif

    // 5. Konfiguracja sprzêtowego Burst Mode (Oversampling 4 próbek)
    ADC0.CTRLB = ADC_SAMPNUM_ACC4_gc;
    ADC0.CTRLD = ADC_INITDLY_DLY16_gc; // Bezpieczne opóŸnienie startowe

    // 6. W³¹czenie przerwañ oraz samego modu³u ADC
    ADC_isr_enable();
    ADC_enable();
}

/*********************************************************************************/
void adc_event_cb_reg( void (*callback)( void ) ) {
    adc_end_conv_event_cb = callback;
}

void ADC_EVENT(void) {
    if ( adcEndConvFlag == ADC_mean_ready ) {
        adcEndConvFlag = ADC_mean_NOT_ready;
        if ( adc_end_conv_event_cb != 0 ) {
            adc_end_conv_event_cb();
        }
    }
}

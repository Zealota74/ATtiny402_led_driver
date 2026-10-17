/*
 * sw_soft_timers.c
 * Przepisany dla ATtiny402 (Implementacja oparta o sprz�towy blok TCB0)
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "rcc.h"
#include "sw_soft_timers.h"

// Deklaracja wyprzedzaj�ca zewn�trznej funkcji obs�ugi wej�� (zgodnie z Twoim kodem)
extern void INPUT_event_get(void);

// Definicja zmiennych licznikowych
volatile static uint16_t milis = 0;
volatile uint16_t softTimer1, softTimer2, softTimer3, softTimer4, softTimer5;
volatile uint16_t debounceTimer, analyzeTimer;

/*********************************************************************************/
void sw_soft_timer_init(void) {
    // 0. W��czenie i skonfigurowanie Timera A (TCA0) z preskalerem 16,
    //    aby dostarcza� zegar dla TCB0 (wymagane w czystym main.c)
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm;

    // 1. Ustawienie trybu pracy Timera B: Periodic Interrupt
    TCB0.CTRLB = TCB_CNTMODE_INT_gc;

    // 2. Konfiguracja �r�d�a zegara: sygna� CLK_PER podzielony przez preskaler Timera A (TCA0 DIV16)
    TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc;

    // 3. AUTOMATYCZNY OKRES (Zale�ny od wybranego F_CPU)
    #if F_CPU == 20000000L
        TCB0.CCMP = 12500; // 20MHz / 16 / 12500 = 100Hz (10ms)
    #elif F_CPU == 16000000L
        TCB0.CCMP = 10000; // 16MHz / 16 / 10000 = 100Hz (10ms)
    #elif F_CPU == 8000000L
        TCB0.CCMP = 5000;  // 8MHz / 16 / 5000   = 100Hz (10ms)
    #elif F_CPU == 3333333L
        TCB0.CCMP = 2083;  // 3.33MHz / 16 / 2083 = 100Hz (10ms)
    #elif F_CPU == 1000000L
        TCB0.CCMP = 625;   // 1MHz / 16 / 625    = 100Hz (10ms)
    #else
        #error "Nieobs�ugiwana cz�stotliwo�� F_CPU w sw_soft_timers.c!"
    #endif

    // 4. Wyczyszczenie flagi przerwania i w��czenie przerwa�
    TCB0.INTFLAGS = TCB_CAPT_bm;
    TCB0.INTCTRL  = TCB_CAPT_bm;

    // 5. Uruchomienie licznika TCB0
    TCB0.CTRLA |= TCB_ENABLE_bm;
}

/*********************************************************************************/
uint32_t millis(void) {
    // W architekturze 8-bitowej odczyt zmiennej 32-bitowej 'milis' musi by� atomowy,
    // aby przerwanie nie zmodyfikowa�o jej w trakcie odczytu kolejnych bajt�w.
    uint32_t milis_copy;
    uint8_t sreg_cache = SREG;
    cli();
    milis_copy = milis;
    SREG = sreg_cache;
    return milis_copy;
}

/********************** Nowy wektor przerwania dla Timera B0 *******************/
ISR( TCB0_INT_vect ) {

    // KRYTYCZNE: Kasowanie flagi przerwania w nowej architekturze poprzez wpisanie '1'
    TCB0.INTFLAGS = TCB_CAPT_bm;

    // Inkrementacja systemowej bazy czasu (wielokrotno�� 10 ms)
    milis = milis + SYSTICK_MS; // Zmieniamy na +10, aby millis() zwraca�o realny czas w milisekundach

    // Dok�adnie Twoja niezmieniona, zoptymalizowana rejestrowo logika soft-timer�w
    register uint16_t n;
    n = softTimer1;
    if (n) softTimer1 = n - SYSTICK_MS;

    n = softTimer2;
    if (n) softTimer2 = n - SYSTICK_MS;

    n = softTimer3;
    if (n) softTimer3 = n - SYSTICK_MS;

    n = softTimer4;
    if (n) softTimer4 = n - SYSTICK_MS;

    n = softTimer5;
    if (n) softTimer5 = n - SYSTICK_MS;

    n = debounceTimer;
    if (n) debounceTimer = n - SYSTICK_MS;

    n = analyzeTimer;
    if (n) analyzeTimer = n - SYSTICK_MS;

    // Wywo�anie Twojego managera zdarze� dla przycisk�w
    INPUT_event_get();
}

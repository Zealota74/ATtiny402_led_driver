/*
 * gpio.c
 * Przepisany dla ATtiny402
 */
#include <avr/io.h>

#include "ATTINY402_led_driver.h"
#include "gpio.h"

/*********************************************************************************/
void gpio_pin_cfg( GPIO_TypeDef * const port, T_GPIO_PIN pin, T_GPIO_MODE mode ) {

    // 1. Ustawienie kierunku (Odpowiednik MODER z STM32 za pomoc� DIRSET / DIRCLR)
    if (mode == gpio_mode_output_PP) {
        port->DIRSET = pin; // Ustaw jako wyj�cie
    } else {
        port->DIRCLR = pin; // Ustaw jako wej�cie
    }

    // 2. Konfiguracja wewn�trznej struktury pinu (Odpowiednik PUPDR z STM32 za pomoc� PINxCTRL)
    // Musimy dobra� si� do rejestru kontrolnego danego pinu. Rejestry te s� u�o�one sekwencyjnie jako tablica.
    // Wyznaczamy indeks pinu na podstawie maski bitowej za pomoc� wbudowanej funkcji GCC __builtin_ctz
    uint8_t pin_index = __builtin_ctz(pin);

    // Wska�nik pomocniczy do rejestru PINxCTRL wybranego pinu
    volatile uint8_t * pin_ctrl_reg = &port->PIN0CTRL + pin_index;

    switch (mode) {
        case gpio_mode_in_PU:
            *pin_ctrl_reg = PORT_PULLUPEN_bm; // W��czenie rezystora podci�gaj�cego (Pull-Up)
            break;

        case gpio_mode_analog:
            *pin_ctrl_reg = PORT_ISC_INPUT_DISABLE_gc; // Wy��czenie bufora cyfrowego dla oszcz�dno�ci pr�du i pracy ADC
            break;

        case gpio_mode_in_floating:
        case gpio_mode_output_PP:
        default:
            *pin_ctrl_reg = PORT_ISC_INTDISABLE_gc; // Brak dodatkowych funkcji, przerwania wy��czone
            break;
    }
}

/*********************************************************************************/
void RCC_gpio_init( void ) {
    /*
     * W STM32 ta funkcja w��cza taktowanie szyny AHB/APB dla GPIO.
     * W ATtiny402 system dystrybucji zegara stale zasila porty wej��/wyj��.
     * Pozostawiamy funkcj� pust�, aby zachowa� struktur� i kompatybilno�� z Twoj� baz� kodu.
     */
}

/*********************************************************************************/
// Odpowiednik rejestru BRR / BSRR z STM32 (Atomowe czyszczenie bitu)
void gpio_pin_LO( GPIO_TypeDef * const port, T_GPIO_PIN pin ) {
    port->OUTCLR = pin;
}

// Odpowiednik rejestru BSRR z STM32 (Atomowe ustawianie bitu)
void gpio_pin_HI( GPIO_TypeDef * const port, T_GPIO_PIN pin ) {
    port->OUTSET = pin;
}

// Odpowiednik negacji rejestru ODR z STM32 za pomoc� dedykowanego rejestru OUTTGL z AVR
void gpio_pin_XOR( GPIO_TypeDef * const port, T_GPIO_PIN pin ) {
    port->OUTTGL = pin;
}

/*********************************************************************************/
void gpio_pin_IMPULSES( GPIO_TypeDef * const port, T_GPIO_PIN pin, uint8_t num ) {
    // Zachowano dok�adnie Twoj� logik� generowania serii impuls�w
    num = 2 * num + 2;
    while ( --num ) {
        if ( num % 2 ) {
            gpio_pin_LO( port, pin );
        } else {
            gpio_pin_HI( port, pin );
        }
    }
}

/*********************************************************************************/
void GPIO_Configure_EXTI( const GPIO_EXTI_t * extiPIN ) {
    // 1. Konfigurujemy pin jako wej�cie z podci�ganiem (zgodnie z Twoim szablonu z STM32)
    gpio_pin_cfg(extiPIN->port, extiPIN->pin, gpio_mode_in_PU);

    // 2. Wyznaczamy indeks pinu (0-7) na podstawie maski bitowej
    uint8_t pin_index = __builtin_ctz(extiPIN->pin);

    // 3. Pobieramy wska�nik do rejestru kontrolnego PINxCTRL wybranego pinu
    volatile uint8_t * pin_ctrl_reg = &extiPIN->port->PIN0CTRL + pin_index;

    // 4. Czy�cimy flag� przerwania, kt�ra mog�a tam wisie� przypadkowo przed konfiguracj�
    // W nowej architekturze AVR flag� przerwania (INTFLAGS) czy�ci si� wpisuj�c w ni� jedynk�!
    extiPIN->port->INTFLAGS = extiPIN->pin;

    // 5. Zapisujemy konfiguracj� wyzwalania zbocza (oraz Pull-Up) do rejestru kontrolnego pinu
    *pin_ctrl_reg = PORT_PULLUPEN_bm | extiPIN->trigger;
}

/*
 * gpio_defs.h
 * Przepisany dla ATtiny402 (AVR 0-series)
 */

#ifndef GPIO_DEFS_H_
#define GPIO_DEFS_H_

#include "ATTINY402_led_driver.h"

// Alias zgodny z STM32 - w AVR rejestry portu s¹ zdefiniowane przez strukturê PORT_t
typedef PORT_t GPIO_TypeDef;

// Definicje trybów pracy pinu (zredukowane do mo¿liwoœci sprzêtowych ATtiny402)
typedef enum {
    gpio_mode_in_floating = 0, // Wejœcie cyfrowe cyfrowe (Hi-Z)
    gpio_mode_in_PU,           // Wejœcie z w³¹czonym Pull-Upem
    gpio_mode_output_PP,       // Wyjœcie Push-Pull
    gpio_mode_analog           // Wy³¹czenie bufora cyfrowego (wejœcie analogowe ADC)
} T_GPIO_MODE;

// Definicje pinów zgodne z bitmaskami (PINx_bm) w nag³ówkach AVR-GCC
typedef enum {
    PA0 = PIN0_bm,
    PA1 = PIN1_bm,
    PA2 = PIN2_bm,
    PA3 = PIN3_bm,
    PA4 = PIN4_bm,
    PA5 = PIN5_bm,
    PA6 = PIN6_bm,
    PA7 = PIN7_bm
} T_GPIO_PIN;

// Definicje trybów wyzwalania przerwania (Odpowiednik FTSR / RTSR z STM32)
typedef enum {
    gpio_exti_disabled   = PORT_ISC_INTDISABLE_gc,  // Przerwania wy³¹czone
    gpio_exti_both_edges = PORT_ISC_BOTHEDGES_gc,   // Oba zbocza
    gpio_exti_rising     = PORT_ISC_RISING_gc,      // Zbocze narastaj¹ce
    gpio_exti_falling    = PORT_ISC_FALLING_gc,     // Zbocze opadaj¹ce
    gpio_exti_level      = PORT_ISC_LEVEL_gc        // Stan niski
} T_GPIO_EXTI_TRIGGER;

// Struktura EXTI dopasowana do architektury AVR, zachowuj¹ca Twój interfejs
typedef struct {
    GPIO_TypeDef *        port;     // WskaŸnik do portu (np. PORTA)
    T_GPIO_PIN            pin;      // Maska pinu (np. PA1)
    T_GPIO_EXTI_TRIGGER   trigger;  // Wybrane zbocze wyzwalaj¹ce
} GPIO_EXTI_t;


#endif /* GPIO_DEFS_H_ */

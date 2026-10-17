# ATtiny402_led_driver

Sterownik LED na mikrokontroler **ATtiny402** (tinyAVR 0-series).

- MCU: `attiny402`
- F_CPU: `3333333UL` (20 MHz / 6, domyślny preskaler)
- Toolchain: avr-gcc + avr-libc
- IDE: Eclipse 2023-12 + AVR Eclipse Plugin (`de.innot.avreclipse`)

## Import do Eclipse

File → Import → Git → Projects from Git → Clone URI → `https://github.com/Zealota74/ATtiny402_led_driver.git`,
następnie „Import existing Eclipse projects”. Ustawienia projektu (MCU, F_CPU, ścieżki include) są w `.project`, `.cproject` i `.settings/`.

## Struktura

- `Attiny402_led_driver.c` – `main()`
- `inc/` – konfiguracja MCU i nagłówki ogólne
- `libs/SW_AVR_PERIPHS/` – peryferia: GPIO, ADC, TIM, RCC (zegar)
- `libs/SW_TIMERS/` – timery programowe
- `libs/SW_INPUT/` – obsługa wejść / klawiatury
- `libs/SW_LEDLIGHT/` – sterowanie LED
- `libs/SW_BOARD/` – miganie diodą debug
- `00_tmp/` – pliki robocze, wyłączone z kompilacji

## Kompilacja z linii poleceń (Linux)

Starsze avr-gcc wymagają pakietu [Microchip ATtiny_DFP](https://packs.download.microchip.com/):

```sh
avr-gcc -B <DFP>/gcc/dev/attiny402 -isystem <DFP>/include -Iinc -Ilibs -Ilibs/SW_AVR_PERIPHS \
  -mmcu=attiny402 -DF_CPU=3333333UL -Os -std=gnu99 ...
```

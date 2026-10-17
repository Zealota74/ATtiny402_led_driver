/*
 * power_down.h
 * Power-Down sleep on a very long press, wake-up on the next press of KEY_PIN.
 * Draft module: not integrated with SW_LEDLIGHT yet (PWM/ADC are not restored).
 */

#ifndef SW_POWER_POWER_DOWN_H_
#define SW_POWER_POWER_DOWN_H_

// RTC clocked from INT1K (1.024 kHz): 1 tick ~ 0.977 ms
#define WAKE_DEBOUNCE_TICKS     30  // debounce of the press that woke the MCU
#define RELEASE_DEBOUNCE_TICKS  30  // debounce after release
#define RELEASE_POLL_TICKS      10  // key polling period while waiting for release

extern void POWER_DOWN_init(void);      // RTC setup + VLONG_PRESS -> POWER_DOWN_request
extern void POWER_DOWN_request(void);   // sleep now, returns after wake-up and debounce

#endif /* SW_POWER_POWER_DOWN_H_ */

/*
 * input.h
 *
 *  Created on: 1 sty 2022
 *      Author: simon
 */

#ifndef LIBS_SW_INPUT_INPUT_H_
#define LIBS_SW_INPUT_INPUT_H_


// ------------------------- New types ---------------------------
typedef void (*COMMAND_FUN_t)( void );
typedef enum {
	EVENT_KEY1_SHORT_PRESS,
	EVENT_KEY1_MEDIUM_PRESS,
	EVENT_KEY1_DOUBLE_PRESS,
	EVENT_KEY1_VLONG_PRESS,
	EVENT_KEY1_REPEAT_PRESS,

	EVENT_NUMBER
} INPUT_EVENT_t;
enum {
	INPUT_KEYB_EVENT, INPUT_IR_EVENT, INPUT_NO_EVENT
};

// ----------------------- Public functions ----------------------
extern void INPUT_init(void);

extern void INPUT_event_get(void);
extern void INPUT_event_execute(void);

extern void INPUT_EVENTS_cb_reg( INPUT_EVENT_t inputEvent,
								 COMMAND_FUN_t input_event_cb );
extern void INPUT_reset(void);	// wołać po każdej przerwie w ciągłości czasu


// ---------------------------------------------------------------

#endif /* LIBS_SW_INPUT_INPUT_H_ */

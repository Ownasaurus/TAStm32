#ifndef INPUTS_H_
#define INPUTS_H_

// How long to wait in ms before input event can be triggered again
#define DEBOUNCETIME 25

// How long in ms a button has to be pressed before it's considered held
#define HOLDTIME 500

#include <stdint.h>

extern uint8_t menuNeedsUpdating;


typedef enum {
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_ENTER,
	BUTTON_SETTINGS
} ButtonType;

#define NUMINPUTS 4


typedef enum {
	IOEVENT_RELEASE,
	IOEVENT_PRESS,
	IOEVENT_HELD,
	IOEVENT_HOLDING,
	IOEVENT_HOLDRELEASE
} IOEvent;

void inputProcess();

#endif

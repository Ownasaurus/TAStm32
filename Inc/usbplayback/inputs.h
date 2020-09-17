#ifndef INPUTS_H_
#define INPUTS_H_

#define DEBOUNCETIME 1

// How long a button has to be pressed before it's considered held
#define HOLDTIME 14


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

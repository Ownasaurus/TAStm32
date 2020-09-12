#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "usbplayback/menu.h"
#include "usbplayback/inputs.h"
#include "main.h"



static void IOevent(unsigned char pin, unsigned char eventType) {
	if (eventType == IOEVENT_PRESS) {
		if (pin == INPUT_UP)
			Menu_Up();
		else if (pin == INPUT_DOWN)
			Menu_Down();
		else if (pin == INPUT_ENTER)
			Menu_Enter();
		//else if (pin == INPUT_HOME)
		//	Menu_Home();
		Menu_Display();
	} else if (eventType == IOEVENT_RELEASE) {
		Menu_Display();
	} else if (eventType == IOEVENT_HOLDING) {
		if (pin == INPUT_UP)
			Menu_HoldUp();
		else if (pin == INPUT_DOWN)
			Menu_HoldDown();
		Menu_Display();
	} else if (eventType == IOEVENT_HOLDRELEASE) {
		Menu_Display();
	}

}

unsigned long inputNextThink = 0;

int gpiodebounce[4] = { 0, 0, 0, 0 };

void inputProcess(void) {

	GPIO_PinState butstate;

	unsigned char i = 0;

	if (uwTick < inputNextThink)
		return;

	inputNextThink = uwTick + 1;

	for (i = 0; i < NUMINPUTS; i++) {

		if (i == INPUT_UP)
			butstate = !HAL_GPIO_ReadPin (SWITCH1_GPIO_Port, SWITCH1_Pin);
		else if (i == INPUT_DOWN)
			butstate = !HAL_GPIO_ReadPin (SWITCH2_GPIO_Port, SWITCH2_Pin);
		else if (i == INPUT_HOME)
			butstate = !HAL_GPIO_ReadPin (SWITCH3_GPIO_Port, SWITCH3_Pin);
		else if (i == INPUT_ENTER)
			butstate = !HAL_GPIO_ReadPin (SWITCH4_GPIO_Port, SWITCH4_Pin);

		else
			butstate = 0;

		// gpiodebounce = 0 when button not pressed,
		// > 0 and < scsettings.debouncetime when debouncing positive edge
		// > scsettings.debouncetime and < scsettings.holdtime when holding
		// = scsettings.holdtime when continuing to hold
		// > scsettings.holdtime when waiting for release
		// > -scsettings.debouncetime and < 0 when debouncing negative edge

		// Button not pressed, check for button
		if (gpiodebounce[i] == 0) {
			if (butstate) {
				IOevent(i, IOEVENT_PRESS);

				// start the counter
				gpiodebounce[i]++;
			}
		}

		// Debouncing positive edge, increment value
		else if (gpiodebounce[i] > 0 && gpiodebounce[i] < DEBOUNCETIME) {
			gpiodebounce[i]++;
		}

		// debounce finished, keep incrementing until hold reached
		else if (gpiodebounce[i] >= DEBOUNCETIME && gpiodebounce[i] < HOLDTIME) {
			// check to see if unpressed
			if (!butstate) {
				IOevent(i, IOEVENT_RELEASE);
				// start the counter
				gpiodebounce[i] = -DEBOUNCETIME;
			}

			else
				gpiodebounce[i]++;
		}
		// Button has been held for a while
		else if (gpiodebounce[i] == HOLDTIME) {
			gpiodebounce[i]++;
		}

		// Button still holding, check for release
		else if (gpiodebounce[i] > HOLDTIME) {
			// Still pressing, do action repeatedly
			if (butstate) {
				if (i == INPUT_UP || i == INPUT_DOWN) // only for up/down buttons
					IOevent(i, IOEVENT_HOLDING);
			}
			// not still pressing, debounce release
			else {
				IOevent(i, IOEVENT_HOLDRELEASE);
				// start the counter
				gpiodebounce[i] = -DEBOUNCETIME;
			}
		}

		// Debouncing negative edge, increment value - will reset when zero is reached
		else if (gpiodebounce[i] < 0) {
			gpiodebounce[i]++;
		}
	}
	Menu_Display();
}

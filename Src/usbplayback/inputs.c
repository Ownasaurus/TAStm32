#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "usbplayback/menu.h"
#include "usbplayback/inputs.h"
#include "main.h"

static void IOevent(ButtonType pin, IOEvent eventType) {
	if (eventType == IOEVENT_PRESS) {
		if (pin == BUTTON_UP)
			Menu_Up();
		else if (pin == BUTTON_DOWN)
			Menu_Down();
		else if (pin == BUTTON_ENTER)
			Menu_Enter();
		else if (pin == BUTTON_SETTINGS)
			Menu_Settings();
	} else if (eventType == IOEVENT_RELEASE) {
	} else if (eventType == IOEVENT_HOLDING) {
		if (pin == BUTTON_UP)
			Menu_HoldUp();
		else if (pin == BUTTON_DOWN)
			Menu_HoldDown();
	} else if (eventType == IOEVENT_HOLDRELEASE) {
	}
	menuNeedsUpdating = 1;
}

int gpiodebounce[4] = { 0, 0, 0, 0 };

//should be run every 1ms by TIM2
void inputProcess(void) {

	GPIO_PinState butstate;

	ButtonType i;

	for (i = 0; i < NUMINPUTS; i++) {

		if (i == BUTTON_UP)
			butstate = !HAL_GPIO_ReadPin (SWITCH1_GPIO_Port, SWITCH1_Pin);
		else if (i == BUTTON_DOWN)
			butstate = !HAL_GPIO_ReadPin (SWITCH2_GPIO_Port, SWITCH2_Pin);
		else if (i == BUTTON_SETTINGS)
			butstate = !HAL_GPIO_ReadPin (SWITCH3_GPIO_Port, SWITCH3_Pin);
		else if (i == BUTTON_ENTER)
			butstate = !HAL_GPIO_ReadPin (SWITCH4_GPIO_Port, SWITCH4_Pin);

		else
			butstate = 0;

		// gpiodebounce = 0 when button not pressed
		// > 0 and < DEBOUNCETIME when debouncing positive edge
		// >= DEBOUNCETIME and < HOLDTIME when waiting for release or hold action
		// = HOLDTIME when we register it as a hold action
		// > HOLDTIME when waiting for release
		// > -DEBOUNCETIME and < 0 when debouncing negative edge

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
				if (i == BUTTON_UP || i == BUTTON_DOWN) // only for up/down buttons
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

}

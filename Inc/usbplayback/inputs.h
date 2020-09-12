/*
 * inputs.h
 *
 *  Created on: 7 Sep 2020
 *      Author: user
 */

#ifndef INPUTS_H_
#define INPUTS_H_

#define INPUT_UP 0
#define INPUT_DOWN 1
#define INPUT_ENTER 2
#define INPUT_HOME 3

#define NUMINPUTS 4

#define IOEVENT_RELEASE 0
#define IOEVENT_PRESS 1
#define IOEVENT_HELD 2
#define IOEVENT_HOLDING 3
#define IOEVENT_HOLDRELEASE 4

#define HOLDTIME 400
#define DEBOUNCETIME 2

void inputProcess(void);

#endif /* INPUTS_H_ */

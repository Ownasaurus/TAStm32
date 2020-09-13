/*
 * usbplayback.h
 *
 *  Created on: 9 Sep 2020
 *      Author: user
 */

#ifndef USBPLAYBACK_USBPLAYBACK_H_
#define USBPLAYBACK_USBPLAYBACK_H_


typedef enum
{
	RUNSTATE_IDLE,
	RUNSTATE_RUNNING,
	RUNSTATE_STOPPING,
	RUNSTATE_STOPPED
} PlaybackState;

extern PlaybackState USBPlaybackState;

void USB_Playback_Init();
void USB_Playback_Task();

#endif /* USBPLAYBACK_USBPLAYBACK_H_ */

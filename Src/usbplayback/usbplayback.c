#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "main.h"
#include "fatfs.h"
#include "ssd1306/ssd1306.h"
#include "usbplayback/usbplayback.h"
#include "usbplayback/menu.h"
#include "usbplayback/inputs.h"
#include "usbd_cdc_if.h"
#include "n64.h"
#include "TASRun.h"
#include "stm32f4xx_it.h"
#include "serial_interface.h"

uint32_t readcount = 0;
uint32_t latchCount;
FIL TasFile;

PlaybackState USBPlaybackState = RUNSTATE_STOPPED;

char inputBuffer[512];
uint32_t inputBufferSize;

// Set up screen, menus etc
uint8_t USB_Playback_Init() {
	if (!ssd1306_Init())
		return 0;
	//ssd1306_TestAll();
	Menu_Init();
	return 1;
}

void USB_Stop_TAS() {
	USBPlaybackState = RUNSTATE_STOPPED;
	f_close(&TasFile);
	ResetRun();
}


void USB_Start_Tas(char *file) {
	FRESULT res;
	TASRun *tasrun = TASRunGetByIndex(RUN_A);
	char *tasfile = NULL;
	char *extension = strrchr(file, '.');

	ResetRun();

	// Extension is tcf, grab parameters from it
	if (strcmp(extension, ".tcf") == 0) {
		if (load_tcf(tasrun, file)) {
			tasfile = tasrun->inputFile;
		}
	}

	// Failing that, guess at some default parameters by the extension name. TODO maybe try and find an associated tcf?
	else if (strcmp(extension, ".r08") == 0) {

		TASRunSetConsole(tasrun, CONSOLE_NES);
		tasfile = file;
		SetSNESMode();
		TASRunSetNumControllers(tasrun, 2);
		TASRunSetNumDataLanes(tasrun, 1);

	} else if (strcmp(extension, ".r16m") == 0) {

		TASRunSetConsole(tasrun, CONSOLE_SNES);
		tasfile = file;
		SetSNESMode();
		TASRunSetNumControllers(tasrun, 2);
		TASRunSetNumDataLanes(tasrun, 4);

	}

	// we want to read in approx 512-byte chunks, but it has to be a multiple of the input data size
	// clamp the input buffer size to the highest integer multiple of the input data to 512
	inputBufferSize = (512 / tasrun->input_data_size) * tasrun->input_data_size;

	if (tasfile != NULL) {
		res = f_open(&TasFile, tasfile, FA_READ);
		if (res == FR_OK) {
			// Add blank frames
			while (tasrun->blank--) {
				ExtractDataAndAddFrame(tasrun, NULL, tasrun->input_data_size);
			}

			USBPlaybackState = RUNSTATE_RUNNING;
		}
	}
}

// Handle all the menus and buffer stuffing, etc
void USB_Playback_Task() {
	static UINT br;
	static uint8_t buffer[512];
	static FRESULT res;
	TASRun *tasrun = TASRunGetByIndex(RUN_A);

	switch (USBPlaybackState) {

	case RUNSTATE_RUNNING:

		// Fill buffer up to the last inputBufferSize
		while (TASRunGetSize(tasrun) * tasrun->input_data_size < (1024 * tasrun->input_data_size) - inputBufferSize) {
			res = f_read(&TasFile, buffer, inputBufferSize, &br);
			if (res == FR_OK && br >= tasrun->input_data_size) {
				readcount += br;
				for (int k = 0; k < br; k += tasrun->input_data_size) {
					ExtractDataAndAddFrame(tasrun, &buffer[k], tasrun->input_data_size);
				}
			} else { // we ran out of data
				USBPlaybackState = RUNSTATE_STOPPING;
				break;
			}
		}

		if (!TASRunIsInitialized(tasrun) && TASRunGetSize(tasrun) > 0) { // this should only run once per run to set up the 1st frame of data

			if (tasrun->console == CONSOLE_NES || tasrun->console == CONSOLE_SNES) {
				if (TASRunGetDPCMFix(tasrun)) {
					toggleNext = 1;
				}
				if (TASRunGetClockFix(tasrun)) {
					clockFix = 1;
				}

				EXTI1_IRQHandler();
			}

			TASRunSetInitialized(tasrun, 1);

			__disable_irq();
			HAL_NVIC_EnableIRQ(EXTI0_IRQn);
			HAL_NVIC_EnableIRQ(EXTI1_IRQn);
			HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
			if (tasrun->multitap)
				HAL_NVIC_EnableIRQ(EXTI4_IRQn);
			__enable_irq();
		}
		break;

	case RUNSTATE_STOPPING:
		if (TASRunGetSize(tasrun) == 0) {
			USB_Stop_TAS();
		}
		break;

	default:
		break;

	}

	inputProcess();
}


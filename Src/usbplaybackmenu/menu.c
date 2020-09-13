#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include "usbplayback/menu.h"
#include "usbplayback/usbplayback.h"
#include "main.h"
#include "fatfs.h"
#include "ssd1306/ssd1306.h"
#include "usbd_cdc_if.h"
#include "n64.h"
#include "TASRun.h"
#include "stm32f4xx_it.h"
#include "serial_interface.h"

extern Menu_Item_t MenuBrowser;

//										Name				Type				Param	Next			Previous			Parent			Child
Menu_Item_t MenuBrowser = { "Browser", MENUTYPE_BROWSER, NULL, NULL, NULL, NULL,
		NULL };
Menu_Item_t MenuRunning = { "Running", MENUTYPE_RUNNINGTAS, NULL, NULL, NULL, NULL,
		NULL };

Menu_Item_t *CurrentMenu;
Menu_Item_t *RootMenu = &MenuBrowser;

#define DISPLAYLINES 8

int16_t cursorPos = 0;
int16_t displayPos = 0;
unsigned char USBok = 0;

char currentFilename[256];

FATFS TASDrive;
extern FIL TasFile;


void Menu_Enter() {
	if (CurrentMenu->Type == MENUTYPE_NORMAL) {
		if (CurrentMenu == NULL)
			return;

		if (CurrentMenu->Child != NULL) {
			CurrentMenu = CurrentMenu->Child;
		}
	} else if (CurrentMenu->Type == MENUTYPE_BROWSER) {
		USB_Start_Tas(&currentFilename[0]);
		CurrentMenu = &MenuRunning;
	} else if (CurrentMenu->Type == MENUTYPE_RUNNINGTAS) {
		USB_Stop_TAS();
		CurrentMenu = &MenuBrowser;
	}
}

void Menu_Up() {
	if (CurrentMenu->Type == MENUTYPE_NORMAL) {
		if (CurrentMenu->Previous != NULL)
			CurrentMenu = CurrentMenu->Previous;
	}

	else if (CurrentMenu->Type == MENUTYPE_BROWSER) {
		if (cursorPos > 0)
			cursorPos--;
	}
}

void Menu_Down() {
	if (CurrentMenu->Type == MENUTYPE_NORMAL) {
		if (CurrentMenu->Next != NULL)
			CurrentMenu = CurrentMenu->Next;
	}

	else if (CurrentMenu->Type == MENUTYPE_BROWSER) {
		cursorPos++;
	}
}

unsigned long stepNextThink = 0;

#define STEPS 33

void Menu_HoldUp() {
	if (uwTick > stepNextThink) {
		Menu_Up();
		stepNextThink = uwTick + STEPS;
	}
}

void Menu_HoldDown() {
	if (uwTick > stepNextThink) {
		Menu_Down();
		stepNextThink = uwTick + STEPS;
	}
}

/*void Menu_Home() {

 if (RootMenu != NULL) {
 if (CurrentMenu == &MenuMain)
 CurrentMenu = &MenuPresets;
 else
 CurrentMenu = &MenuMain;
 }
 }*/
extern uint32_t selswaps;
void Menu_Display() {
	static char line[23];
	static char temp[23];
	int chars;
	//LCD_CLS();
	//ssd1306_clear();

	if (CurrentMenu->Type == MENUTYPE_BROWSER) {
		static FRESULT res;
		static DIR dir;
		static char path[2] = "/";
		static FILINFO fno;

		unsigned char lineNo = 0;

		if (USBok) {
			res = f_opendir(&dir, &path[0]);

			ssd1306_Fill(Black);
			if (res == FR_OK) {
				if (cursorPos > (displayPos + 7)) {
					displayPos = cursorPos - 7;
				} else if (cursorPos < displayPos) {
					displayPos = cursorPos;
				}
				//Iterate through every directory entry till we get to the ones we need
				// Perhaps could cache this but would require a lot of RAM
				for (uint16_t cnt = 0; cnt < displayPos + DISPLAYLINES; cnt++) {
					res = f_readdir(&dir, &fno);
					if (res != FR_OK || fno.fname[0] == 0) {
						displayPos--;
						cursorPos--;
						break;
					}
					if (cnt >= displayPos) {
						SSD1306_COLOR textColor = White;
						if (cnt == cursorPos) {
							textColor = Black;
							strcpy(currentFilename,
									fno.fname[0] == '\0' ?
											fno.altname : fno.fname);
						}
						ssd1306_SetCursor(0, lineNo * 8);
						ssd1306_WriteString(
								fno.fname[0] == '\0' ? fno.altname : fno.fname,
								Font_6x8, textColor);
						lineNo++;
					}
				}
				ssd1306_UpdateScreen();
			}
		} else {
			res = f_mount(&TASDrive, (TCHAR const*) USBHPath, 1);
			if (res == FR_OK) {
				USBok = 1;

			} else {
				ssd1306_Fill(Black);
				ssd1306_SetCursor(0, 0);
				sprintf(temp, "No USB");
				ssd1306_WriteString(temp, Font_16x26, White);
				ssd1306_UpdateScreen();
			}
		}
	}

	else if (CurrentMenu->Type == MENUTYPE_RUNNINGTAS) {
		TASRun *tasrun = TASRunGetByIndex(RUN_A);

		ssd1306_Fill(Black);

		/*sprintf(textBuf, "Disk Reads:%d", readcount);
		 ssd1306_SetCursor(0, 0);
		 ssd1306_WriteString(textBuf, Font_6x8, White);

		 sprintf(textBuf, "Buffer:%d", tasrun->size);
		 ssd1306_SetCursor(0, 8);
		 ssd1306_WriteString(textBuf, Font_6x8, White);

		 sprintf(textBuf, "Latches:%d", latchCount);
		 ssd1306_SetCursor(0, 16);
		 ssd1306_WriteString(textBuf, Font_6x8, White);*/

		RunDataArray *ct = tasrun->current;

		switch (tasrun->console) {

		case CONSOLE_NES:

			ssd1306_SetCursor(0, 25);
			ssd1306_WriteString("L", Font_16x26,
					ct[0][0]->nes_data.left ? Black : White);

			ssd1306_SetCursor(16, 12);
			ssd1306_WriteString("U", Font_16x26,
					ct[0][0]->nes_data.up ? Black : White);

			ssd1306_SetCursor(16, 38);
			ssd1306_WriteString("D", Font_16x26,
					ct[0][0]->nes_data.down ? Black : White);

			ssd1306_SetCursor(32, 25);
			ssd1306_WriteString("R", Font_16x26,
					ct[0][0]->nes_data.right ? Black : White);

			ssd1306_SetCursor(48, 25);
			ssd1306_WriteString("s", Font_16x26,
					ct[0][0]->nes_data.select ? Black : White);

			ssd1306_SetCursor(64, 25);
			ssd1306_WriteString("S", Font_16x26,
					ct[0][0]->nes_data.start ? Black : White);

			ssd1306_SetCursor(80, 25);
			ssd1306_WriteString("B", Font_16x26,
					ct[0][0]->nes_data.b ? Black : White);

			ssd1306_SetCursor(96, 25);
			ssd1306_WriteString("A", Font_16x26,
					ct[0][0]->nes_data.a ? Black : White);
			break;

		case CONSOLE_SNES:

			ssd1306_SetCursor(0, 25);
			ssd1306_WriteString("L", Font_16x26,
					ct[0][0]->snes_data.left ? Black : White);

			ssd1306_SetCursor(16, 12);
			ssd1306_WriteString("U", Font_16x26,
					ct[0][0]->snes_data.up ? Black : White);

			ssd1306_SetCursor(16, 38);
			ssd1306_WriteString("D", Font_16x26,
					ct[0][0]->snes_data.down ? Black : White);

			ssd1306_SetCursor(32, 25);
			ssd1306_WriteString("R", Font_16x26,
					ct[0][0]->snes_data.right ? Black : White);

			ssd1306_SetCursor(48, 25);
			ssd1306_WriteString("s", Font_16x26,
					ct[0][0]->snes_data.select ? Black : White);

			ssd1306_SetCursor(64, 25);
			ssd1306_WriteString("S", Font_16x26,
					ct[0][0]->snes_data.start ? Black : White);

			ssd1306_SetCursor(80, 25);
			ssd1306_WriteString("Y", Font_16x26,
					ct[0][0]->snes_data.y ? Black : White);

			ssd1306_SetCursor(96, 12);
			ssd1306_WriteString("X", Font_16x26,
					ct[0][0]->snes_data.x ? Black : White);

			ssd1306_SetCursor(112, 25);
			ssd1306_WriteString("A", Font_16x26,
					ct[0][0]->snes_data.a ? Black : White);

			ssd1306_SetCursor(96, 38);
			ssd1306_WriteString("B", Font_16x26,
					ct[0][0]->snes_data.b ? Black : White);

			ssd1306_SetCursor(0, 0);
			ssd1306_WriteString("<", Font_16x26,
					ct[0][0]->snes_data.l ? Black : White);

			ssd1306_SetCursor(112, 0);
			ssd1306_WriteString(">", Font_16x26,
					ct[0][0]->snes_data.l ? Black : White);

			break;
		}

		/*sprintf(textBuf, "fucked:%d", fuckedLatches);
		 ssd1306_SetCursor(0, 50);
		 ssd1306_WriteString(textBuf, Font_6x8, White);*/

		ssd1306_UpdateScreen();

		if (USBPlaybackState == RUNSTATE_STOPPED){
			CurrentMenu = &MenuBrowser;
		}
	}

	else {
		ssd1306_Fill(Black);
		ssd1306_SetCursor(0, 0);
		ssd1306_WriteString(CurrentMenu->Name, Font_6x8, White);
		ssd1306_UpdateScreen();
		/*sprintf(line, "ss:%d", selswaps);
		 ssd1306_Fill(Black);
		 ssd1306_SetCursor(0, 0);
		 ssd1306_WriteString(line, Font_6x8, White);
		 ssd1306_UpdateScreen();
		 */
	}
}
void Menu_Init() {
	CurrentMenu = &MenuBrowser;
}


#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>

typedef enum
{
	CONSOLE_N64,
	CONSOLE_SNES,
	CONSOLE_NES
} Console;

typedef struct
{
	Console console;
	uint8_t numControllers;
} TASRun;

void ResetTASRuns();
void TASRunSetNumControllers(int numRun, uint8_t numControllers);
void TASRunSetConsole(int numRun, Console c);

#endif

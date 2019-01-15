#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>
#include "n64.h"

typedef enum
{
	CONSOLE_N64,
	CONSOLE_SNES,
	CONSOLE_NES
} Console;


typedef union
{
	N64ControllerData n64_data[1024];
} TASData;

typedef struct
{
	Console console;
	uint8_t numControllers;
	TASData runData;
	TASData *buf; // points to the next place the received serial data will be stored
	TASData *end; // points to the end of the array for bounds checking
	TASData *current; // points to what the n64 will read next
} TASRun;

void ResetTASRuns();
void TASRunSetNumControllers(int numRun, uint8_t numControllers);
void TASRunSetConsole(int numRun, Console c);
uint8_t AddN64Frame(int runIndex, N64ControllerData* frame);
N64ControllerData GetNextN64Frame(int runNum);

#endif

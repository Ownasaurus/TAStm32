#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>
#include "n64.h"
#include "snes.h"

#define MAX_SIZE 1024

typedef enum
{
	CONSOLE_N64,
	CONSOLE_SNES,
	CONSOLE_NES
} Console;


typedef union
{
	N64ControllerData n64_data;
	SNESControllerData snes_data;
	NESControllerData nes_data;
} RunData;

typedef struct
{
	Console console;
	uint8_t numControllers;
	RunData runData[MAX_SIZE];
	RunData *buf; // points to the next place the received serial data will be stored
	RunData *end; // points to the end of the array for bounds checking
	RunData *current; // points to what the n64 will read next
	uint8_t bit; // only used for NES/SNES
	uint16_t size;
	uint8_t runStarted;
} TASRun;

uint8_t GetRunStarted(int numRun);
void SetRunStarted(int numRun, uint8_t started);
void ResetTASRuns();
void TASRunSetNumControllers(int numRun, uint8_t numControllers);
void TASRunSetConsole(int numRun, Console c);
uint8_t AddFrame(int runIndex, RunData* frame);
Console TASRunGetConsole(int numRun);
void GetRunDataAndAdvance(RunData* rd, int index);
void ExtractDataAndAdvance(RunData* frame, int index, uint8_t* Buf, int *byteNum);
RunData* GetNextFrame(int runNum);
N64ControllerData* GetNextN64Frame(int runNum);

#endif

#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>
#include "n64.h"
#include "snes.h"

#define MAX_SIZE 1024
#define MAX_CONTROLLERS 2
#define MAX_DATA_LANES 3

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
	uint8_t numDataLanes;
	RunData runData[MAX_SIZE][MAX_CONTROLLERS][MAX_DATA_LANES];
	RunData (*buf)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to the next place the received serial data will be stored
	RunData (*end)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to the end of the array for bounds checking
	RunData (*current)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to what the n64 will read next
	uint8_t bit; // only used for NES/SNES
	uint16_t size;
	uint8_t runStarted;
} TASRun;

uint8_t GetRunStarted(int numRun);
void SetRunStarted(int numRun, uint8_t started);
void ResetTASRuns();
void TASRunSetNumControllers(int numRun, uint8_t numControllers);
uint8_t TASRunGetNumControllers(int numRun);
void TASRunSetNumDataLanes(int numRun, uint8_t numDataLanes);
uint8_t TASRunGetNumDataLanes(int numRun);
void TASRunSetConsole(int numRun, Console c);
uint8_t AddFrame(int runIndex, RunData (*frame)[MAX_CONTROLLERS][MAX_DATA_LANES]);
Console TASRunGetConsole(int numRun);
void GetRunDataAndAdvance(RunData (*rd)[MAX_CONTROLLERS][MAX_DATA_LANES], int index);
void ExtractDataAndAdvance(RunData (*rd)[MAX_CONTROLLERS][MAX_DATA_LANES], int index, uint8_t* Buf, int *byteNum);
RunData (*GetNextFrame(int runNum))[MAX_CONTROLLERS][MAX_DATA_LANES];
void SetP1Data0InputMode();
void SetP1Data0OutputMode();

#endif

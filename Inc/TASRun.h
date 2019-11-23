#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>
#include "n64.h"
#include "snes.h"

#define MAX_SIZE 1024
#define MAX_CONTROLLERS 2
#define MAX_DATA_LANES 3
#define MAX_TRANSITIONS 5

typedef enum
{
	CONSOLE_N64,
	CONSOLE_SNES,
	CONSOLE_NES,
	CONSOLE_GC
} Console;


typedef union
{
	N64ControllerData n64_data;
	SNESControllerData snes_data;
	NESControllerData nes_data;
	GCControllerData gc_data;
} RunData;

typedef enum
{
	TRANSITION_NORMAL,
	TRANSITION_ACE,
	TRANSITION_RESET_SOFT,
	TRANSITION_RESET_HARD
} TransitionType;

typedef struct
{
	TransitionType type;
	uint32_t frameno;
} Transition;

typedef struct
{
	Console console;
	uint8_t numControllers;
	uint8_t numDataLanes;
	RunData runData[MAX_SIZE][MAX_CONTROLLERS][MAX_DATA_LANES];
	RunData (*buf)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to the next place the received serial data will be stored
	RunData (*end)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to the end of the array for bounds checking
	RunData (*current)[MAX_CONTROLLERS][MAX_DATA_LANES]; // points to what the console will read next
	volatile uint16_t size;
	uint8_t dpcmFix;
	uint8_t clockFix;
	uint8_t overread;
	uint8_t initialized;
	volatile uint32_t frameCount;
	Transition transitions_dpcm[MAX_TRANSITIONS];
	uint8_t console_data_size;
	uint8_t input_data_size;
} TASRun;

TASRun *TASRunGetByIndex(uint8_t runNum);
uint16_t TASRunGetSize(const TASRun *tasrun);
uint8_t TASRunIsInitialized(const TASRun *tasrun);
void TASRunSetInitialized(TASRun *tasrun, uint8_t init);
uint8_t AddTransition(TASRun *tasrun, TransitionType type, uint32_t frameNumber);
void ResetTASRuns();
void TASRunSetDPCMFix(TASRun *tasrun, uint8_t dpcm);
uint8_t TASRunGetDPCMFix(const TASRun *tasrun);
void TASRunSetClockFix(TASRun *tasrun, uint8_t cf);
uint8_t TASRunGetClockFix(const TASRun *tasrun);
void TASRunSetOverread(TASRun *tasrun, uint8_t overread);
uint8_t TASRunGetOverread(const TASRun *tasrun);
void TASRunSetNumControllers(TASRun *tasrun, uint8_t numControllers);
uint8_t TASRunGetNumControllers(const TASRun *tasrun);
void TASRunSetNumDataLanes(TASRun *tasrun, uint8_t numDataLanes);
uint8_t TASRunGetNumDataLanes(const TASRun *tasrun);
void TASRunSetConsole(TASRun *tasrun, Console console);
uint32_t TASRunGetFrameCount(const TASRun *tasrun);
uint8_t TASRunIncrementFrameCount(TASRun *tasrun);
uint8_t AddFrame(TASRun *tasrun, RunData (frame)[MAX_CONTROLLERS][MAX_DATA_LANES]);
Console TASRunGetConsole(const TASRun *tasrun);
void ExtractDataAndAdvance(RunData (rd)[MAX_CONTROLLERS][MAX_DATA_LANES], TASRun *tasrun, uint8_t* Buf, int *byteNum);
RunData (*GetNextFrame(TASRun *tasrun))[MAX_CONTROLLERS][MAX_DATA_LANES];
int ExtractDataAndAddFrame(TASRun *tasrun, uint8_t *buffer, uint32_t n);
uint8_t GetSizeOfInputForRun(const TASRun *tasrun);
void UpdateSizeOfInputForRun(TASRun *tasrun);

void SetN64InputMode();
void SetN64OutputMode();

void SetN64Mode();
void SetSNESMode();

#endif

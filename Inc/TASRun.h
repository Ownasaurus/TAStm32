#ifndef __TASRUN__H
#define __TASRUN__H

#include "main.h"
#include "n64.h"
#include "snes.h"

#define MAX_SIZE 1024
#define MAX_CONTROLLERS 2
#define MAX_DATA_LANES 3
#define MAX_TRANSITIONS 5

#define MAX_NUM_RUNS 2

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

typedef RunData RunDataArray[MAX_CONTROLLERS][MAX_DATA_LANES];

typedef struct
{
	Console console;
	uint8_t numControllers;
	uint8_t numDataLanes;
	RunDataArray runData[MAX_SIZE];
	RunDataArray *buf; // points to the next place the received serial data will be stored
	RunDataArray *end; // points to the end of the array for bounds checking
	RunDataArray *current; // points to what the console will read next
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

extern TASRun tasruns[MAX_NUM_RUNS];

// This file includes a load of static functions with definitions to make
// inlining easier


maybe_unused static TASRun *TASRunGetByIndex(uint8_t runNum)
{
	return &tasruns[runNum];
}


maybe_unused static uint16_t TASRunGetSize(const TASRun *tasrun)
{
	return tasrun->size;
}

maybe_unused static uint8_t TASRunIsInitialized(const TASRun *tasrun)
{
	return tasrun->initialized;
}

maybe_unused static void TASRunSetInitialized(TASRun *tasrun, uint8_t init)
{
	tasrun->initialized = init;
}

maybe_unused static uint32_t TASRunGetFrameCount(const TASRun *tasrun)
{
	return tasrun->frameCount;
}

maybe_unused static void TASRunSetOverread(TASRun *tasrun, uint8_t overread)
{
	tasrun->overread = overread;
}

maybe_unused static uint8_t TASRunGetOverread(const TASRun *tasrun)
{
	return tasrun->overread;
}

maybe_unused static void TASRunSetDPCMFix(TASRun *tasrun, uint8_t dpcm)
{
	tasrun->dpcmFix = dpcm;
}

maybe_unused static uint8_t TASRunGetDPCMFix(const TASRun *tasrun)
{
	return tasrun->dpcmFix;
}

maybe_unused static uint8_t TASRunGetNumControllers(const TASRun *tasrun)
{
	return tasrun->numControllers;
}

maybe_unused static uint8_t TASRunGetNumDataLanes(const TASRun *tasrun)
{
	return tasrun->numDataLanes;
}

maybe_unused static Console TASRunGetConsole(const TASRun *tasrun)
{
	return tasrun->console;
}

maybe_unused static uint8_t GetSizeOfInputForRun(const TASRun *tasrun)
{
	return tasrun->input_data_size;
}

// Functions below here are complex enough to not try to inline

uint8_t AddTransition(TASRun *tasrun, TransitionType type, uint32_t frameNumber);
void ResetTASRuns();


void TASRunSetClockFix(TASRun *tasrun, uint8_t cf);
uint8_t TASRunGetClockFix(const TASRun *tasrun);


// These three functions update the cached 'input_data_size'
void TASRunSetNumControllers(TASRun *tasrun, uint8_t numControllers);
void TASRunSetNumDataLanes(TASRun *tasrun, uint8_t numDataLanes);
void TASRunSetConsole(TASRun *tasrun, Console console);

uint8_t TASRunIncrementFrameCount(TASRun *tasrun);
uint8_t AddFrame(TASRun *tasrun, RunDataArray frame);
void ExtractDataAndAdvance(RunDataArray rd, TASRun *tasrun, uint8_t* Buf, int *byteNum);
RunDataArray *GetNextFrame(TASRun *tasrun);
int ExtractDataAndAddFrame(TASRun *tasrun, uint8_t *buffer, uint32_t n);

void SetN64Mode();
void SetSNESMode();
#endif

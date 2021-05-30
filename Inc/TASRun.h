#ifndef __TASRUN__H
#define __TASRUN__H

#include <stdint.h>
#include "n64.h"
#include "snes.h"
#include "gen.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define MAX_SIZE 1024
#define MAX_CONTROLLERS 2
#define MAX_DATA_LANES 4
#define MAX_TRANSITIONS 5


typedef enum
{
	CONSOLE_N64,
	CONSOLE_SNES,
	CONSOLE_NES,
	CONSOLE_GC,
	CONSOLE_GEN
} Console;


typedef union
{
	N64ControllerData n64_data;
	SNESControllerData snes_data;
	NESControllerData nes_data;
	GCControllerData gc_data;
	GENControllerData gen_data;
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
	uint32_t moder_firstLatch;
	uint8_t meleeMitigation;
	uint32_t pollNumber;

	uint8_t multitap;
	int32_t blank;
	char inputFile[256];
} TASRun;

extern TASRun *tasrun;
extern TASRun tasruns;

// This file includes a load of static functions with definitions to make
// inlining easier

#define maybe_unused  __attribute__((unused))

maybe_unused static void SetN64InputMode(uint8_t player)
{
	if(player == 1)
	{
		// port C4 to input mode
		const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_INPUT * MODER_SLOT;
		P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
	}
	else if(player == 2)
	{
		const uint32_t MODER_SLOT = (P2_DATA_2_Pin*P2_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_INPUT * MODER_SLOT;
		P2_DATA_2_GPIO_Port->MODER = (P2_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
	}
}

maybe_unused static void SetN64OutputMode(uint8_t player)
{
	if(player == 1)
	{
		// port C4 to output mode
		const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_OUTPUT_PP * MODER_SLOT;
		P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
	}
	else if(player == 2)
	{
		const uint32_t MODER_SLOT = (P2_DATA_2_Pin*P2_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_OUTPUT_PP * MODER_SLOT;
		P2_DATA_2_GPIO_Port->MODER = (P2_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
	}
}

// Functions below here are complex enough to not try to inline

uint8_t AddTransition(TransitionType type, uint32_t frameNumber);
void ResetTASRuns();


void TASRunSetClockFix(uint8_t cf);
uint8_t TASRunGetClockFix();


// These three functions update the cached 'input_data_size'
void TASRunSetNumControllers(uint8_t numControllers);
void TASRunSetNumDataLanes(uint8_t numDataLanes);
void TASRunSetConsole(Console console);

uint8_t TASRunIncrementFrameCount();
uint8_t AddFrame(RunDataArray frame);
void ExtractDataAndAdvance(RunDataArray rd, uint8_t* Buf, int *byteNum);
RunDataArray *GetNextFrame();
int ExtractDataAndAddFrame(uint8_t *buffer, uint32_t n);

void SetN64Mode();
void SetSNESMode();
void SetGENMode();
void SetMultitapMode();
void ResetRun();
#endif

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

#ifdef BOARDV3

#define SWITCH1_Pin GPIO_PIN_6
#define SWITCH1_GPIO_Port GPIOA

#define SWITCH2_Pin GPIO_PIN_7
#define SWITCH2_GPIO_Port GPIOA

#define SWITCH3_Pin GPIO_PIN_14
#define SWITCH3_GPIO_Port GPIOC

#define SWITCH4_Pin GPIO_PIN_13
#define SWITCH4_GPIO_Port GPIOC

#endif //BOARDV3

#ifdef BOARDV4

#define SWITCH4_Pin GPIO_PIN_12
#define SWITCH4_GPIO_Port GPIOB

#define SWITCH3_Pin GPIO_PIN_13
#define SWITCH3_GPIO_Port GPIOB

#define SWITCH2_Pin GPIO_PIN_8
#define SWITCH2_GPIO_Port GPIOA

#define SWITCH1_Pin GPIO_PIN_10
#define SWITCH1_GPIO_Port GPIOA

#endif //BOARDV4


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
	TRANSITION_RESET_HARD,
	TRANSITION_WAIT_RUMBLE
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
	uint8_t controller_mode;
	uint8_t meleeMitigation;
	uint32_t pollNumber;
	uint8_t waiting; // if we're waiting on a rumble

	uint8_t multitap;
	int32_t blank;
	char inputFile[256];
} TASRun;

extern TASRun *tasrun;
extern TASRun tasruns;

// This file includes a load of static functions with definitions to make
// inlining easier

#define maybe_unused  __attribute__((unused))
void SetupPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t Mode, uint32_t Pull, GPIO_PinState PinState);
maybe_unused static void SetN64InputMode(uint8_t player)
{
	#ifdef BOARDV3
	if(player == 1)
	{
		// port C4 to input mode
		const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_INPUT * MODER_SLOT;
		P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
		// with a pullup
		P1_DATA_2_GPIO_Port->PUPDR = (P1_DATA_2_GPIO_Port->PUPDR & ~MODER_MASK) | (0b01 * MODER_SLOT);
	}
	else if(player == 2)
	{
		// port C9 to input mode
		const uint32_t MODER_SLOT = (P2_DATA_2_Pin*P2_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = GPIO_MODE_INPUT * MODER_SLOT;
		P2_DATA_2_GPIO_Port->MODER = (P2_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
		// with a pullup
		P2_DATA_2_GPIO_Port->PUPDR = (P2_DATA_2_GPIO_Port->PUPDR & ~MODER_MASK) | (0b01 * MODER_SLOT);
	}
	#endif
	// v4 board is always in input and output mode simultaneously
}

maybe_unused static void SetN64OutputMode(uint8_t player)
{
	#ifdef BOARDV3
	if(player == 1)
	{
		// port C4 to output mode
		const uint32_t MODER_SLOT = (P1_DATA_2_Pin*P1_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = /*GPIO_MODE_OUTPUT_PP * */MODER_SLOT;
		P1_DATA_2_GPIO_Port->OTYPER |= (1 << 4); // set to open drain
		P1_DATA_2_GPIO_Port->MODER = (P1_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
		// with a pullup
		P1_DATA_2_GPIO_Port->PUPDR = (P1_DATA_2_GPIO_Port->PUPDR & ~MODER_MASK) | (0b01 * MODER_SLOT);
	}
	else if(player == 2)
	{
		// port C9 to output mode
		const uint32_t MODER_SLOT = (P2_DATA_2_Pin*P2_DATA_2_Pin);
		const uint32_t MODER_MASK = 0b11 * MODER_SLOT;
		const uint32_t MODER_NEW_VALUE = /*GPIO_MODE_OUTPUT_PP * */MODER_SLOT;
		P2_DATA_2_GPIO_Port->OTYPER |= (1 << 9); // set to open drain
		P2_DATA_2_GPIO_Port->MODER = (P2_DATA_2_GPIO_Port->MODER & ~MODER_MASK) | MODER_NEW_VALUE;
		// with a pullup
		P2_DATA_2_GPIO_Port->PUPDR = (P2_DATA_2_GPIO_Port->PUPDR & ~MODER_MASK) | (0b01 * MODER_SLOT);
	}
	#endif

	// v4 board is always in input and output mode simultaneously
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
void SetNESMode();
void SetSNESMode();
void SetGENMode();
void SetMultitapMode();
void ResetRun();
void ResetGPIO(void);

#endif

#include <string.h>
#include "n64.h"
#include "snes.h"
#include "TASRun.h"
#include "stm32f4xx_hal.h"
#include "main.h"

#define MAX_NUM_RUNS 2

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

TASRun tasruns[MAX_NUM_RUNS];

uint16_t TASRunGetSize(uint8_t runNum)
{
	return tasruns[runNum].size;
}

uint8_t TASRunIsInitialized(uint8_t runNum)
{
	return tasruns[runNum].initialized;
}

void TASRunSetInitialized(uint8_t runNum, uint8_t init)
{
	tasruns[runNum].initialized = init;
}

RunData (*GetNextFrame(int runNum))[MAX_CONTROLLERS][MAX_DATA_LANES]
{
	if(tasruns[runNum].size == 0) // in case of buffer underflow
	{
		return NULL; // buffer underflow
	}

	RunData (*retval)[MAX_CONTROLLERS][MAX_DATA_LANES] = (RunData*)tasruns[runNum].current;

	// advance frame
	if(tasruns[runNum].current != tasruns[runNum].end)
	{
		(tasruns[runNum].current)++;
	}
	else
	{
		tasruns[runNum].current = tasruns[runNum].runData;
	}

	tasruns[runNum].size--;

	return retval;
}

uint8_t AddTransition(int numRun, TransitionType type, uint32_t frameNumber)
{
	int x = 0;
	while(x < MAX_TRANSITIONS)
	{
		if(tasruns[numRun].transitions_dpcm[x].frameno == 0) // first blank transition slot found
		{
			tasruns[numRun].transitions_dpcm[x].frameno = frameNumber;
			tasruns[numRun].transitions_dpcm[x].type = type;
			return 1;
		}

		x++;
	}

	return 0; // failure: no room to add transition
}

uint32_t TASRunGetFrameCount(int numRun)
{
	return tasruns[numRun].frameCount;
}

uint8_t TASRunIncrementFrameCount(int numRun)
{
	tasruns[numRun].frameCount++;

	int x = 0;
	while(x < MAX_TRANSITIONS)
	{
		if(tasruns[numRun].transitions_dpcm[x].frameno == 0) // out of transitions to search for
		{
			break;
		}

		if(tasruns[numRun].transitions_dpcm[x].frameno == tasruns[numRun].frameCount)
		{
			switch(tasruns[numRun].transitions_dpcm[x].type)
			{
				case TRANSITION_ACE:
					tasruns[numRun].dpcmFix = 0;
					return 1;
				break;
				case TRANSITION_NORMAL:
					tasruns[numRun].dpcmFix = 1;
					return 1;
				break;
				case TRANSITION_RESET_SOFT:
					return 2;
				break;
				case TRANSITION_RESET_HARD:
					return 3;
				break;
			}
		}

		x++;
	}

	return 0;
}

void TASRunSetOverread(int numRun, uint8_t overread)
{
	tasruns[numRun].overread = overread;
}

uint8_t TASRunGetOverread(int numRun)
{
	return tasruns[numRun].overread;
}

void TASRunSetDPCMFix(int numRun, uint8_t dpcm)
{
	tasruns[numRun].dpcmFix = dpcm;
}

uint8_t TASRunGetDPCMFix(int numRun)
{
	return tasruns[numRun].dpcmFix;
}

void TASRunSetClockFix(int numRun, uint8_t cf)
{
	if(cf > 1)
	{
		tasruns[numRun].clockFix = cf;
		htim6.Init.Period = htim7.Init.Period = cf-1;
	}
	else
	{
		tasruns[numRun].clockFix = 0;
	}
}

uint8_t TASRunGetClockFix(int numRun)
{
	return (tasruns[numRun].clockFix != 0) ? 1 : 0;
}

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
	for(int x = 0;x < MAX_NUM_RUNS;x++)
	{
		tasruns[x].buf = tasruns[x].runData;
		tasruns[x].current = tasruns[x].runData;
		tasruns[x].end = &(tasruns[x].runData[MAX_SIZE-1]);
	}
}

void TASRunSetNumControllers(int numRun, uint8_t numControllers)
{
	tasruns[numRun].numControllers = numControllers;
}

uint8_t TASRunGetNumControllers(int numRun)
{
	return tasruns[numRun].numControllers;
}

void TASRunSetNumDataLanes(int numRun, uint8_t numDataLanes)
{
	tasruns[numRun].numDataLanes = numDataLanes;
}

uint8_t TASRunGetNumDataLanes(int numRun)
{
	return tasruns[numRun].numDataLanes;
}

Console TASRunGetConsole(int numRun)
{
	return tasruns[numRun].console;
}

void TASRunSetConsole(int numRun, Console console)
{
	tasruns[numRun].console = console;
}

void ExtractDataAndAdvance(RunData (rd)[MAX_CONTROLLERS][MAX_DATA_LANES], int index, uint8_t* Buf, int *byteNum)
{
	uint8_t bytesPerInput = 0;
	uint8_t numControllers = tasruns[index].numControllers;
	uint8_t numDataLanes = tasruns[index].numDataLanes;

	memset(rd, 0, sizeof(RunData[MAX_CONTROLLERS][MAX_DATA_LANES])); // prepare the data container

	switch(tasruns[index].console)
	{
		case CONSOLE_N64:
			bytesPerInput = sizeof(N64ControllerData);
			break;
		case CONSOLE_SNES:
			bytesPerInput = sizeof(SNESControllerData);
			break;
		case CONSOLE_NES:
			bytesPerInput = sizeof(NESControllerData);
			break;
		default: // should never reach this
			break;
	}

	for(int x = 0;x < numControllers;x++)
	{
		for(int y = 0;y < numDataLanes;y++)
		{
			memcpy(&rd[x][y], &(Buf[(*byteNum)]), bytesPerInput); // copy only what is necessary
			(*byteNum) += bytesPerInput; // advance the index only what is necessary
		}
	}

	(*byteNum)--; // back up 1 since the main loop will advance it one
}

uint8_t AddFrame(int runIndex, RunData (frame)[MAX_CONTROLLERS][MAX_DATA_LANES])
{
	// first check buffer isn't full
	if(tasruns[runIndex].size == MAX_SIZE)
	{
		return 0;
	}

	memcpy((RunData*)tasruns[runIndex].buf,frame,sizeof(RunData[MAX_CONTROLLERS][MAX_DATA_LANES]));

	// NOTE: These two pointer modifications must occur in an atomic fashion
	//       A poorly-timed interrupt could cause bad things.
	__disable_irq();
	// loop around if necessary
	if(tasruns[runIndex].buf != tasruns[runIndex].end)
	{
		(tasruns[runIndex].buf)++;
	}
	else // buf is at end, so wrap around to beginning
	{
		tasruns[runIndex].buf = tasruns[runIndex].runData;
	}

	tasruns[runIndex].size++;
	__enable_irq();

	return 1;
}

void SetN64InputMode()
{
	// port C4 to input mode
	GPIOC->MODER &= ~(1 << 9);
	GPIOC->MODER &= ~(1 << 8);
}

void SetN64OutputMode()
{
	// port C4 to output mode
	GPIOC->MODER &= ~(1 << 9);
	GPIOC->MODER |= (1 << 8);
}

void SetN64Mode()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = P1_DATA_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SetSNESMode()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = P1_DATA_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

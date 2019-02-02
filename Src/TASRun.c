#include <string.h>
#include "n64.h"
#include "snes.h"
#include "TASRun.h"
#include "stm32f4xx_hal.h"

#define MAX_NUM_RUNS 2

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

	RunData (*retval)[MAX_CONTROLLERS][MAX_DATA_LANES] = tasruns[runNum].current;

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

uint8_t AddTransition(int numRun, uint32_t frameNumber)
{
	int x = 0;
	while(x < MAX_TRANSITIONS)
	{
		if(tasruns[numRun].transitions_dpcm[x] == 0) // first blank transition slot found
		{
			tasruns[numRun].transitions_dpcm[x] = frameNumber;
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
		if(tasruns[numRun].transitions_dpcm[x] == 0) // out of transitions to search for
		{
			break;
		}

		if(tasruns[numRun].transitions_dpcm[x] == tasruns[numRun].frameCount)
		{
			tasruns[numRun].dpcmFix = 1 - tasruns[numRun].dpcmFix;
			return 1;
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

	memcpy(tasruns[runIndex].buf,frame,sizeof(RunData[MAX_CONTROLLERS][MAX_DATA_LANES]));

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

	return 1;
}

void SetP1Data0InputMode()
{
	// port A8 to input mode
	GPIOA->MODER &= ~(1 << 17);
	GPIOA->MODER &= ~(1 << 16);
}

void SetP1Data0OutputMode()
{
	// port A8 to output mode
	GPIOA->MODER &= ~(1 << 17);
	GPIOA->MODER |= (1 << 16);
}

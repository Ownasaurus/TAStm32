#include <string.h>
#include "n64.h"
#include "snes.h"
#include "TASRun.h"
#include "stm32f4xx_hal.h"

TASRun tasruns[4];

RunData* GetNextFrame(int runNum)
{
	if(tasruns[runNum].size == 0) // in case of buffer underflow
	{
		return NULL; // buffer underflow
	}

	RunData* retval = tasruns[runNum].current;

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

N64ControllerData* GetNextN64Frame(int runNum)
{
	if(tasruns[runNum].size == 0) // in case of buffer underflow
	{
		return NULL;
	}

	N64ControllerData* retval = (N64ControllerData*)tasruns[runNum].current;

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

uint8_t GetRunStarted(int numRun)
{
	return tasruns[numRun].runStarted;
}

void SetRunStarted(int numRun, uint8_t started)
{
	tasruns[numRun].runStarted = started;
}

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
	for(int x = 0;x < 4;x++)
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

Console TASRunGetConsole(int numRun)
{
	return tasruns[numRun].console;
}

void TASRunSetConsole(int numRun, Console console)
{
	tasruns[numRun].console = console;
}

void GetRunDataAndAdvance(RunData* rd, int index)
{
	memcpy(rd,tasruns[0].current,sizeof(*rd));
	(tasruns[index].current)++;
}

void ExtractDataAndAdvance(RunData* rd, int index, uint8_t* Buf, int *byteNum)
{
	uint8_t bytesPerFrame = 0;
	uint8_t maxPlayers = 0;

	memset(rd, 0, sizeof(*rd)); // prepare the data container

	switch(tasruns[index].console)
	{
		case CONSOLE_N64:
			bytesPerFrame = sizeof(N64ControllerData);
			maxPlayers = 1;
			break;
		case CONSOLE_SNES:
			bytesPerFrame = sizeof(SNESControllerData);
			maxPlayers = 4;
			break;
		case CONSOLE_NES:
			bytesPerFrame = sizeof(NESControllerData);
			maxPlayers = 2;
			break;
		default: // should never reach this
			break;
	}

	memcpy(rd, &(Buf[(*byteNum)]), bytesPerFrame); // copy only what is necessary
	(*byteNum) += ((bytesPerFrame * maxPlayers) - 1); // advance the index only what is necessary
}

uint8_t AddFrame(int runIndex, RunData* frame)
{
	// first check buffer isn't full
	if(tasruns[runIndex].size == MAX_SIZE)
	{
		return 0;
	}

	memcpy(tasruns[runIndex].buf,frame,sizeof(*frame));

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

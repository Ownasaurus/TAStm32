#include <string.h>
#include "n64.h"
#include "TASRun.h"

TASRun tasruns[4];

N64ControllerData GetNextN64Frame(int runNum)
{
	if(tasruns[runNum].size <= 0) // in case of buffer underflow
	{
		N64ControllerData blank;
		memset(&blank,0,sizeof(N64ControllerData));
		return blank; // send blank controller data
	}

	N64ControllerData* retval = tasruns[runNum].current;

	if(tasruns[runNum].current != tasruns[runNum].end)
	{
		(tasruns[runNum].current)++;
	}
	else
	{
		tasruns[runNum].current = tasruns[runNum].runData;
	}

	tasruns[runNum].size--;

	return *retval;
}

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
	for(int x = 0;x < 4;x++)
	{
		tasruns[x].end = &(tasruns[x].runData[MAX_SIZE-1]);
		tasruns[x].buf = tasruns[x].runData;
		tasruns[x].current = tasruns[x].runData;
	}
}

void TASRunSetNumControllers(int numRun, uint8_t numControllers)
{
	tasruns[numRun].numControllers = numControllers;
}

void TASRunSetConsole(int numRun, Console console)
{
	tasruns[numRun].console = console;
}

uint8_t AddN64Frame(int runIndex, N64ControllerData* frame)
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

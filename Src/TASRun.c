#include <string.h>
#include "n64.h"
#include "TASRun.h"

TASRun tasruns[4];

N64ControllerData GetNextN64Frame(int runNum)
{
	N64ControllerData* retval = tasruns[runNum].current;

	if(tasruns[runNum].buf != tasruns[runNum].end)
	{
		(tasruns[runNum].buf)++;
	}
	else
	{
		(tasruns[runNum].buf) = tasruns[runNum].runData.n64_data;
	}

	return *retval;
}

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
	for(int x = 0;x < 4;x++)
	{
		tasruns[x].end = &(tasruns[x].runData.n64_data[1024]);
		tasruns[x].buf = &(tasruns[x].runData);
		tasruns[x].current = &(tasruns[x].runData);
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
	if(tasruns[runIndex].buf == (tasruns[runIndex].current)-1)
	{
		return 0;
	}

	memcpy(tasruns[runIndex].buf,frame,sizeof(frame));

	// loop around if necessary
	if(tasruns[runIndex].buf != tasruns[runIndex].end)
	{
		(tasruns[runIndex].buf)++;
	}
	else
	{
		tasruns[runIndex].buf = tasruns[runIndex].runData.n64_data;
	}

	return 1;
}

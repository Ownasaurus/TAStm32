#include <string.h>
#include "n64.h"
#include "TASRun.h"

TASRun tasruns[4];

N64ControllerData GetNextN64Frame(int runNum)
{
	//TODO confirm pointer math works

	N64ControllerData* retval = (N64ControllerData*)tasruns[runNum].current;

	if(tasruns[runNum].current != tasruns[runNum].end)
	{
		(tasruns[runNum].current)++;
	}
	else
	{
		tasruns[runNum].current = tasruns[runNum].runData;
	}

	return *retval;
}

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
	for(int x = 0;x < 4;x++)
	{
		tasruns[x].end = &(tasruns[x].runData[1024]);
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
	// TODO: confirm pointer math works

	// first check buffer isn't full
	if(tasruns[runIndex].buf == (tasruns[runIndex].current)-1)
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

	return 1;
}

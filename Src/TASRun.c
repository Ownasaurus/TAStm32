#include <string.h>
#include "TASRun.h"

TASRun tasruns[4];

void ResetTASRuns()
{
	memset(tasruns,0,sizeof(tasruns));
}

void TASRunSetNumControllers(int numRun, uint8_t numControllers)
{
	tasruns[numRun].numControllers = numControllers;
}

void TASRunSetConsole(int numRun, Console console)
{
	tasruns[numRun].console = console;
}

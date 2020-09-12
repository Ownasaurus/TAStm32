#include <string.h>
#include "n64.h"
#include "snes.h"
#include "TASRun.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "main.h"

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

TASRun tasruns[MAX_NUM_RUNS];

RunDataArray *GetNextFrame(TASRun *tasrun)
{
	if(tasrun->size == 0) // in case of buffer underflow
	{
		return NULL; // buffer underflow
	}

	RunDataArray *retval = tasrun->current;

	// advance frame
	if(tasrun->current != tasrun->end)
	{
		(tasrun->current)++;
	}
	else
	{
		tasrun->current = tasrun->runData;
	}

	tasrun->size--;

	return retval;
}

uint8_t AddTransition(TASRun *tasrun, TransitionType type, uint32_t frameNumber)
{
	int x = 0;
	while(x < MAX_TRANSITIONS)
	{
		if(tasrun->transitions_dpcm[x].frameno == 0) // first blank transition slot found
		{
			tasrun->transitions_dpcm[x].frameno = frameNumber;
			tasrun->transitions_dpcm[x].type = type;
			return 1;
		}

		x++;
	}

	return 0; // failure: no room to add transition
}

uint8_t TASRunIncrementFrameCount(TASRun *tasrun)
{
	tasrun->frameCount++;

	int x = 0;
	while(x < MAX_TRANSITIONS)
	{
		if(tasrun->transitions_dpcm[x].frameno == 0) // out of transitions to search for
		{
			break;
		}

		if(tasrun->transitions_dpcm[x].frameno == tasrun->frameCount)
		{
			switch(tasrun->transitions_dpcm[x].type)
			{
				case TRANSITION_ACE:
					tasrun->dpcmFix = 0;
					return 1;
				break;
				case TRANSITION_NORMAL:
					tasrun->dpcmFix = 1;
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


void TASRunSetClockFix(TASRun *tasrun, uint8_t cf)
{
	if(cf > 1)
	{
		tasrun->clockFix = cf;
		htim6.Init.Period = htim7.Init.Period = cf-1;
	}
	else
	{
		tasrun->clockFix = 0;
	}
}

uint8_t TASRunGetClockFix(const TASRun *tasrun)
{
	return (tasrun->clockFix != 0) ? 1 : 0;
}

void ClearRunData() {
	memset(tasruns, 0, sizeof(tasruns));
	for (int x = 0; x < MAX_NUM_RUNS; x++) {
		tasruns[x].buf = tasruns[x].runData;
		tasruns[x].current = tasruns[x].runData;
		tasruns[x].end = &(tasruns[x].runData[MAX_SIZE - 1]);
	}
}

void ResetRun() {
	// disable interrupts on latch/clock/data for now
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	HAL_NVIC_DisableIRQ(EXTI1_IRQn);
	HAL_NVIC_DisableIRQ(EXTI4_IRQn);
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
	Disable8msTimer();
	DisableP1ClockTimer();
	DisableP2ClockTimer();
	DisableTrainTimer();
	// clear all interrupts
	while (HAL_NVIC_GetPendingIRQ(EXTI0_IRQn)) {
		__HAL_GPIO_EXTI_CLEAR_IT(P1_CLOCK_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI1_IRQn)) {
		__HAL_GPIO_EXTI_CLEAR_IT(P1_LATCH_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI4_IRQn)) {
		__HAL_GPIO_EXTI_CLEAR_IT(P1_DATA_2_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI9_5_IRQn)) {
		__HAL_GPIO_EXTI_CLEAR_IT(P2_CLOCK_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM3_IRQn)) {
		HAL_NVIC_ClearPendingIRQ(TIM3_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM6_DAC_IRQn)) {
		HAL_NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM7_IRQn)) {
		HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM1_UP_TIM10_IRQn)) {
		HAL_NVIC_ClearPendingIRQ(TIM1_UP_TIM10_IRQn);
	}
	// set all lines low to avoid conflicting with NES poweron
	HAL_GPIO_WritePin(GPIOC, P1_DATA_1_Pin | P1_DATA_0_Pin | P2_DATA_1_Pin | P2_DATA_0_Pin | P2_DATA_2_Pin | P1_DATA_2_Pin, GPIO_PIN_RESET);
	// important to reset our state
	recentLatch = 0;
	toggleNext = 0;
	p1_current_bit = 0;
	p2_current_bit = 0;
	dpcmFix = 0;
	clockFix = 0;
	request_pending = 0;
	bulk_mode = 0;
	current_train_index = 0;
	current_train_latch_count = 0;
	between_trains = 0;
	trains_enabled = 0;
	if (latch_trains != NULL) {
		free(latch_trains);
		latch_trains = NULL;
	}
	memset(P1_GPIOC_current, 0, sizeof(P1_GPIOC_current));
	memset(P1_GPIOC_next, 0, sizeof(P1_GPIOC_next));
	memset(P2_GPIOC_current, 0, sizeof(P2_GPIOC_current));
	memset(P2_GPIOC_next, 0, sizeof(P2_GPIOC_next));
	memset(V1_GPIOB_current, 0, sizeof(V1_GPIOB_current));
	memset(V1_GPIOB_next, 0, sizeof(V1_GPIOB_next));
	memset(V2_GPIOC_current, 0, sizeof(V2_GPIOC_current));
	memset(V2_GPIOC_next, 0, sizeof(V2_GPIOC_next));
	ClearRunData();
}


static void UpdateSizeOfInputForRun(TASRun *tasrun)
{
	tasrun->input_data_size = tasrun->numControllers * tasrun->numDataLanes * tasrun->console_data_size;
}

void TASRunSetNumControllers(TASRun *tasrun, uint8_t numControllers)
{
	tasrun->numControllers = numControllers;
	UpdateSizeOfInputForRun(tasrun);
}



void TASRunSetNumDataLanes(TASRun *tasrun, uint8_t numDataLanes)
{
	tasrun->numDataLanes = numDataLanes;
	UpdateSizeOfInputForRun(tasrun);
}



void TASRunSetConsole(TASRun *tasrun, Console console)
{
	tasrun->console = console;

	switch (console)
	{
		case CONSOLE_N64:
			tasrun->console_data_size = sizeof(N64ControllerData);
			break;
		case CONSOLE_SNES:
			tasrun->console_data_size = sizeof(SNESControllerData);
			break;
		case CONSOLE_NES:
			tasrun->console_data_size = sizeof(NESControllerData);
			break;
		case CONSOLE_GC:
			tasrun->console_data_size = sizeof(GCControllerData);
			break;
	}
	UpdateSizeOfInputForRun(tasrun);
}



int ExtractDataAndAddFrame(TASRun *tasrun, uint8_t *buffer, uint32_t n)
{
	size_t bytesPerInput = tasrun->console_data_size;
	uint8_t numControllers = tasrun->numControllers;
	uint8_t numDataLanes = tasrun->numDataLanes;

	RunDataArray frame;

	memset(frame, 0, sizeof(frame)); // prepare the data container

	uint8_t *buffer_position = buffer;
	for(int x = 0;x < numControllers;x++)
	{
		for(int y = 0;y < numDataLanes;y++)
		{
			memcpy(&frame[x][y], buffer_position, bytesPerInput); // copy only what is necessary
			buffer_position += bytesPerInput; // advance the index only what is necessary
		}
	}

	if(tasrun->size == MAX_SIZE)
	{
		return 0;
	}

	memcpy(tasrun->buf,frame,sizeof(*(tasrun->buf)));

	// NOTE: These two pointer modifications must occur in an atomic fashion
	//       A poorly-timed interrupt could cause bad things.
	__disable_irq();
	// loop around if necessary
	if(tasrun->buf != tasrun->end)
	{
		(tasrun->buf)++;
	}
	else // buf is at end, so wrap around to beginning
	{
		tasrun->buf = tasrun->runData;
	}

	tasrun->size++;
	__enable_irq();

	return 1;
}

void ExtractDataAndAdvance(RunDataArray rd, TASRun *tasrun, uint8_t* Buf, int *byteNum)
{
	uint8_t bytesPerInput = 0;
	uint8_t numControllers = tasrun->numControllers;
	uint8_t numDataLanes = tasrun->numDataLanes;

	memset(rd, 0, sizeof(RunDataArray)); // prepare the data container

	switch(tasrun->console)
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
		case CONSOLE_GC:
			bytesPerInput = sizeof(GCControllerData);
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

uint8_t AddFrame(TASRun *tasrun, RunDataArray frame)
{
	// first check buffer isn't full
	if(tasrun->size == MAX_SIZE)
	{
		return 0;
	}

	memcpy((RunData*)tasrun->buf,frame,sizeof(RunDataArray));

	// NOTE: These two pointer modifications must occur in an atomic fashion
	//       A poorly-timed interrupt could cause bad things.
	__disable_irq();
	// loop around if necessary
	if(tasrun->buf != tasrun->end)
	{
		(tasrun->buf)++;
	}
	else // buf is at end, so wrap around to beginning
	{
		tasrun->buf = tasrun->runData;
	}

	tasrun->size++;
	__enable_irq();

	return 1;
}

void SetN64Mode()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = P1_DATA_2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(P1_DATA_2_GPIO_Port, &GPIO_InitStruct);
}

void SetSNESMode()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = P1_DATA_0_Pin|P1_DATA_1_Pin|P2_DATA_0_Pin|P2_DATA_1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


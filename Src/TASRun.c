#include <stdlib.h>
#include <string.h>
#include "n64.h"
#include "snes.h"
#include "gen.h"
#include "TASRun.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "main.h"

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

// Local definition of tasrun structure
TASRun tasruns;

// Global pointer to it
TASRun *tasrun = &tasruns;

extern RunDataArray *dataptr;

RunDataArray *GetNextFrame()
{
	if(tasrun->controller_mode == 1)
	{
		return tasrun->runData;
	}
	if (tasrun->size == 0) // in case of buffer underflow
	{
		return NULL; // buffer underflow
	}

	RunDataArray *retval = tasrun->current;

	// advance frame
	if (tasrun->current != tasrun->end)
	{
		(tasrun->current)++;
	} else
	{
		tasrun->current = tasrun->runData;
	}

	tasrun->size--;

	return retval;
}

uint8_t AddTransition(TransitionType type, uint32_t frameNumber)
{
	int x = 0;
	while (x < MAX_TRANSITIONS)
	{
		if (tasrun->transitions_dpcm[x].frameno == 0) // first blank transition slot found
		{
			tasrun->transitions_dpcm[x].frameno = frameNumber;
			tasrun->transitions_dpcm[x].type = type;
			return 1;
		}

		x++;
	}

	return 0; // failure: no room to add transition
}

uint8_t TASRunIncrementFrameCount()
{
	tasrun->frameCount++;

	int x = 0;
	while (x < MAX_TRANSITIONS)
	{
		if (tasrun->transitions_dpcm[x].frameno == 0) // out of transitions to search for
		{
			break;
		}

		if (tasrun->transitions_dpcm[x].frameno == tasrun->frameCount)
		{
			switch (tasrun->transitions_dpcm[x].type)
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
			case TRANSITION_WAIT_RUMBLE:
				return 4;
				break;
			}
		}

		x++;
	}

	return 0;
}

void TASRunSetClockFix(uint8_t cf)
{
	if (cf > 1)
	{
		tasrun->clockFix = cf;
		htim6.Init.Period = htim7.Init.Period = cf - 1;
	} else
	{
		tasrun->clockFix = 0;
	}
}

uint8_t TASRunGetClockFix()
{
	return (tasrun->clockFix != 0) ? 1 : 0;
}

void ClearRunData()
{
	memset(&tasruns, 0, sizeof(tasruns));
	tasruns.buf = tasruns.runData;
	tasruns.current = tasruns.runData;
	tasruns.end = &(tasruns.runData[MAX_SIZE - 1]);
}


// Get ports into a "neutral" state, before any console-specific setup takes place
void ResetGPIO(void){

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	// all GPIO pins by default should be input, with a pullup to stop them floating
	// 85FF/34FB/FFCF chosen to avoid messing with peripheral pins
	SetupPin(GPIOA, 0x85FF, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_RESET);
	SetupPin(GPIOB, 0x34FB, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_RESET);
	SetupPin(GPIOC, 0xFFCF, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_RESET);

	// tristate all external buffers on v4 board (i.e. set their enable pins high)
	#ifdef BOARDV4
	SetupPin(ENABLE_D0D1_GPIO_Port, ENABLE_D0D1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(ENABLE_P1D2D3_GPIO_Port, ENABLE_P1D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(ENABLE_P2D2D3_GPIO_Port, ENABLE_P2D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	#endif

	// Switches, input pullup (just to be explicit)
	SetupPin(SWITCH1_GPIO_Port, SWITCH1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(SWITCH2_GPIO_Port, SWITCH2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(SWITCH3_GPIO_Port, SWITCH3_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(SWITCH4_GPIO_Port, SWITCH4_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);

	/*Configure Visualization pins: V1_CLOCK_Pin V1_LATCH_Pin V1_DATA_1_Pin V1_DATA_0_Pin */
	SetupPin(V1_DATA_0_GPIO_Port, V1_CLOCK_Pin|V1_LATCH_Pin|V1_DATA_1_Pin|V1_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_RESET);

	/*Configure Visualization pins: V2_CLOCK_Pin V2_LATCH_Pin V2_DATA_1_Pin V2_DATA_0_Pin */
	SetupPin(V2_DATA_0_GPIO_Port, V2_LATCH_Pin|V2_DATA_1_Pin|V2_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_RESET);
	SetupPin(V2_CLOCK_GPIO_Port, V2_CLOCK_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_RESET);

	/*Configure Console Reset pin: */
	SetupPin(SNES_RESET_GPIO_Port, SNES_RESET_Pin, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_PIN_SET);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	//HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	HAL_NVIC_SetPriority(EXTI1_IRQn, 1, 0);
	//HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	//HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}



void ResetRun()
{
	// Reset all GPIOS to poweron state
	ResetGPIO();

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
	while (HAL_NVIC_GetPendingIRQ(EXTI0_IRQn))
	{
		__HAL_GPIO_EXTI_CLEAR_IT(P1_CLOCK_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI1_IRQn))
	{
		__HAL_GPIO_EXTI_CLEAR_IT(P1_LATCH_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI4_IRQn))
	{
		__HAL_GPIO_EXTI_CLEAR_IT(P1_DATA_2_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(EXTI9_5_IRQn))
	{
		__HAL_GPIO_EXTI_CLEAR_IT(P2_CLOCK_Pin);
		__HAL_GPIO_EXTI_CLEAR_IT(P2_DATA_2_Pin);
		HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM3_IRQn))
	{
		HAL_NVIC_ClearPendingIRQ(TIM3_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM6_DAC_IRQn))
	{
		HAL_NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM7_IRQn))
	{
		HAL_NVIC_ClearPendingIRQ(TIM7_IRQn);
	}
	while (HAL_NVIC_GetPendingIRQ(TIM1_UP_TIM10_IRQn))
	{
		HAL_NVIC_ClearPendingIRQ(TIM1_UP_TIM10_IRQn);
	}

	// important to reset our state
	multitapSel = 1;
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
	firstLatch = 1;
	dataptr = 0;
	if (latch_trains != NULL)
	{
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

	// Reset callback functions that may have been overridden
	volatile uint32_t* ptr = (void *)(SRAM_BASE + 0x40 + (EXTI1_IRQn * 4));
	*ptr = (uint32_t) &EXTI1_IRQHandler;

	ptr = (void *)(SRAM_BASE + 0x40 + (EXTI4_IRQn * 4));
	*ptr = (uint32_t) &EXTI4_IRQHandler;

	ptr = (void *)(SRAM_BASE + 0x40 + (EXTI9_5_IRQn * 4));
	*ptr = (uint32_t) &EXTI9_5_IRQHandler;
}

static void UpdateRunConfig()
{
	tasrun->input_data_size = tasrun->numControllers * tasrun->numDataLanes * tasrun->console_data_size;
	tasrun->moder_firstLatch = 0;
	#ifdef BOARDV3
	

	// special case for genesis, which uses 6 data lines for 1 controller
	if(tasrun->console == CONSOLE_GEN)
	{
		tasrun->moder_firstLatch 	|= P1_DATA_0_Pin * P1_DATA_0_Pin
									| P1_DATA_1_Pin * P1_DATA_1_Pin
									| P1_DATA_2_Pin * P1_DATA_2_Pin
									| P2_DATA_0_Pin * P2_DATA_0_Pin
									| P2_DATA_1_Pin * P2_DATA_1_Pin
									| P2_DATA_2_Pin * P2_DATA_2_Pin;

		return;
	}

	// Calculate MODER register for first latch, set appropriate D pins to output
	tasrun->moder_firstLatch |= P1_DATA_0_Pin * P1_DATA_0_Pin; // D0 is always output
	if (tasrun->numDataLanes >= 2) tasrun->moder_firstLatch |= P1_DATA_1_Pin * P1_DATA_1_Pin;
	if (tasrun->numDataLanes == 3 && tasrun->console == CONSOLE_NES) tasrun->moder_firstLatch |= P1_DATA_2_Pin * P1_DATA_2_Pin; // Only for NES, SNES' 3rd data lane is for multitap

	if (tasrun->numControllers == 2){
		tasrun->moder_firstLatch |= P2_DATA_0_Pin * P2_DATA_0_Pin; // D0 is always output
		if (tasrun->numDataLanes >= 2) tasrun->moder_firstLatch |= P2_DATA_1_Pin * P2_DATA_1_Pin;
		if (tasrun->numDataLanes == 3 && tasrun->console == CONSOLE_NES) tasrun->moder_firstLatch |= P2_DATA_2_Pin * P2_DATA_2_Pin; // Only for NES, SNES' 3rd data lane is for multitap
	}
	#endif //BOARDV3
}

void TASRunSetNumControllers(uint8_t numControllers)
{
	tasrun->numControllers = numControllers;
	UpdateRunConfig();
}

void TASRunSetNumDataLanes(uint8_t numDataLanes)
{
	tasrun->numDataLanes = numDataLanes;
	UpdateRunConfig();
}

void TASRunSetConsole(Console console)
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
	case CONSOLE_GEN:
		tasrun->console_data_size = sizeof(GENControllerData);
		break;
	}
	UpdateRunConfig();
}

int ExtractDataAndAddFrame(uint8_t *buffer, uint32_t n)
{
	size_t bytesPerInput = tasrun->console_data_size;
	uint8_t numControllers = tasrun->numControllers;
	uint8_t numDataLanes = tasrun->numDataLanes;

	RunDataArray frame;

	memset(frame, 0, sizeof(frame)); // prepare the data container

	uint8_t *buffer_position = buffer;
	for (int x = 0; x < numControllers; x++)
	{
		for (int y = 0; y < numDataLanes; y++)
		{
			// Null values mean blank frame
			if (buffer_position == NULL)
			{
				memset(&frame[x][y], 0x00, bytesPerInput);
			} else
			{
				memcpy(&frame[x][y], buffer_position, bytesPerInput); // copy only what is necessary
				buffer_position += bytesPerInput; // advance the index only what is necessary
			}
		}
	}

	if (tasrun->controller_mode == 1) // ignore buffer. place at beginning of array
	{
		memcpy(tasrun->runData, frame, sizeof(frame));
		tasrun->size = 1;
		return 1;
	}
	if (tasrun->size == MAX_SIZE)
	{
		return 0;
	}

	memcpy(tasrun->buf, frame, sizeof(*(tasrun->buf)));

	// NOTE: These two pointer modifications must occur in an atomic fashion
	//       A poorly-timed interrupt could cause bad things.
	__disable_irq();
	// loop around if necessary
	if (tasrun->buf != tasrun->end)
	{
		(tasrun->buf)++;
	} else // buf is at end, so wrap around to beginning
	{
		tasrun->buf = tasrun->runData;
	}

	tasrun->size++;
	__enable_irq();

	return 1;
}

void SetN64Mode()
{
	// MCU D2 input, triggered on falling edge
	#ifdef BOARDV3
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin | P2_DATA_2_Pin, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_PIN_SET);
	#endif

	#ifdef BOARDV4

	// MCU D2 input, triggered on falling edge
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin | P2_DATA_2_Pin, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_PIN_SET);

	// Set D2/D3 out LOW before anything else, since we're driving open drain (by wiggling enable)
	SetupPin(P1_DATA_2_OUT_GPIO_Port, P1_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_RESET);
	SetupPin(P2_DATA_2_OUT_GPIO_Port, P2_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_RESET);

	// Buffers will already be disabled
	#endif

	volatile uint32_t* ptr = (void *)(SRAM_BASE + 0x40 + (EXTI4_IRQn * 4));
	*ptr = (uint32_t) &GCN64_P1_Callback;

	ptr = (void *)(SRAM_BASE + 0x40 + (EXTI9_5_IRQn * 4));
	*ptr = (uint32_t) &GCN64_P2_Callback;
}

// quick function to init pin
void SetupPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint32_t Mode, uint32_t Pull, GPIO_PinState PinState){

	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, PinState);
	memset (&GPIO_InitStruct, 0, sizeof(GPIO_InitTypeDef));
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = Mode;
	GPIO_InitStruct.Pull = Pull;
	#ifdef BOARDV3
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	#endif
	#ifdef BOARDV4
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	#endif
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);

}

void SetNESMode()
{

	#ifdef BOARDV3
	// MCU D0/D1/D2 input for now, will be made output on first latch
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_GPIO_Port, P2_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	#endif //BOARDV3

	#ifdef BOARDV4
	// external output buffers should already be tristated, so set direction to output (i.e. high)
	// Buffer will be enabled on first latch
	SetupPin(DIR_D0D1_GPIO_Port, DIR_D0D1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P1D2D3_GPIO_Port, DIR_P1D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P2D2D3_GPIO_Port, DIR_P2D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);

	// Now do the actual MCU pins
	// MCU D0/D1/D2/D3 output, but they will not actually speak to console yet as buffer is disabled
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_2_OUT_GPIO_Port, P1_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_3_GPIO_Port, P1_DATA_3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_OUT_GPIO_Port, P2_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_3_GPIO_Port, P2_DATA_3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	#endif //BOARDV4

	// MCU Clock/latch input, triggered on rising edge
	SetupPin(GPIOC, P1_CLOCK_Pin|P1_LATCH_Pin|P2_CLOCK_Pin, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_PIN_RESET);

	// Ensure the proper callback function is used for pin 1
	volatile uint32_t* ptr = (void *)(SRAM_BASE + 0x40 + (EXTI1_IRQn * 4));
	*ptr = (uint32_t) &NesSnesLatch;
}

void SetSNESMode()
{

	#ifdef BOARDV3
	// MCU D0/D1 input for now, will be made output on first latch
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	
	// MCU D2 may be input or output depending on application, so set input for the moment
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_GPIO_Port, P2_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	#endif //BOARD V3

	#ifdef BOARDV4

	// external output buffers should already be tristated, so set direction to output (i.e. high)
	// Buffer will be enabled on first latch
	SetupPin(DIR_D0D1_GPIO_Port, DIR_D0D1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P1D2D3_GPIO_Port, DIR_P1D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P2D2D3_GPIO_Port, DIR_P2D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);

	// Now do the actual MCU pins
	// MCU D0/D1 output, but they will not actually speak to console yet as buffer is disabled
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);

	// MCU D2 input, handled through the input buffers
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_GPIO_Port, P2_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_PIN_SET);

	#endif //BOARDV4

	// MCU Clock/latch input, triggered on rising edge
	SetupPin(GPIOC, P1_CLOCK_Pin|P1_LATCH_Pin|P2_CLOCK_Pin, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_PIN_RESET);

	// Ensure the proper callback function is used for pin 1
	volatile uint32_t* ptr = (void *)(SRAM_BASE + 0x40 + (EXTI1_IRQn * 4));
	*ptr = (uint32_t) &NesSnesLatch;
}

void SetGENMode()
{
	// P1_D0 --> GEN Pin 1
	// P1_D1 --> GEN Pin 2
	// P1_D2 --> GEN Pin 3
	// P2_D0 --> GEN Pin 4
	// P2_D1 --> GEN Pin 6
	// P2_D2 --> GEN Pin 9
	#ifdef BOARDV3

	// MCU D0/D1/D2 input for now, will be made output on first latch
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_GPIO_Port, P2_DATA_2_Pin, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_PIN_SET);
	
	// Latch pin only interrupts on falling to avoid triggering when console powers on
	// will be set to rising-falling after first interrupt
	SetupPin(P1_LATCH_GPIO_Port, P1_LATCH_Pin, GPIO_MODE_IT_FALLING, GPIO_PULLUP, GPIO_PIN_RESET);
	#endif

	#ifdef BOARDV4

	// external output buffers should already be tristated, so set direction to output (i.e. high)
	// Buffer will be enabled on first latch
	SetupPin(DIR_D0D1_GPIO_Port, DIR_D0D1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P1D2D3_GPIO_Port, DIR_P1D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(DIR_P2D2D3_GPIO_Port, DIR_P2D2D3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);

	// MCU D0/D1/D2/D3 output, but they will not actually speak to console yet as buffer is disabled
	SetupPin(P1_DATA_0_GPIO_Port, P1_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_1_GPIO_Port, P1_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_2_OUT_GPIO_Port, P1_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P1_DATA_3_GPIO_Port, P1_DATA_3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_0_GPIO_Port, P2_DATA_0_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_1_GPIO_Port, P2_DATA_1_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_2_OUT_GPIO_Port, P2_DATA_2_OUT_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);
	SetupPin(P2_DATA_3_GPIO_Port, P2_DATA_3_Pin, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_PIN_SET);

	
	// Latch pin only interrupts on falling to avoid triggering when console powers on
	// will be set to rising-falling after first interrupt
	SetupPin(P1_LATCH_GPIO_Port, P1_LATCH_Pin, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_PIN_RESET);

	#endif

	// Ensure the proper callback function is used for pin 1
	volatile uint32_t* ptr = (void *)(SRAM_BASE + 0x40 + (EXTI1_IRQn * 4));
	*ptr = (uint32_t) &GenesisLatch;
}

void SetMultitapMode()
{
	// If multitap, D2 becomes an interrupt input
	SetupPin(P1_DATA_2_GPIO_Port, P1_DATA_2_Pin, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_PIN_SET);
}

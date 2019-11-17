#include "serial_interface.h"

#include "stm32f4xx_it.h"
#include "main.h"

#include <string.h>

// TODO: replace with atomics?
extern volatile uint8_t p1_current_bit;
extern volatile uint8_t p2_current_bit;
extern volatile uint8_t recentLatch;
extern volatile uint8_t toggleNext;
extern volatile uint8_t dpcmFix;
extern volatile uint8_t clockFix;
extern volatile uint32_t P1_GPIOC_current[32];
extern volatile uint32_t P1_GPIOC_next[32];
extern volatile uint32_t P2_GPIOC_current[32];
extern volatile uint32_t P2_GPIOC_next[32];
extern volatile uint32_t V1_GPIOB_current[16];
extern volatile uint32_t V1_GPIOB_next[16];
extern volatile uint32_t V2_GPIOC_current[16];
extern volatile uint32_t V2_GPIOC_next[16];
extern volatile uint8_t jumpToDFU;
extern const uint8_t SNES_RESET_HIGH_A;
extern const uint8_t SNES_RESET_LOW_A;

extern volatile uint8_t request_pending;
extern volatile uint8_t bulk_mode;

// only instance of this, but make callers use access functions
static serial_interface_state_t instance;

static uint8_t NullOutputFunction(uint8_t *buffer, uint16_t n) {
	return 0;
}

void serial_interface_reset() {
	instance.state = SERIAL_COMPLETE;
	instance.selected_run = RUN_NONE;
	serial_interface_set_output_function(NullOutputFunction);
}

void serial_interface_set_output_function(OutputFunction func)
{
	instance.output_func = func;
}

uint8_t serial_interface_output(uint8_t *buffer, uint16_t n)
{
	return instance.output_func(buffer, n);
}

void serial_interface_consume(uint8_t input)
{
	switch(instance.state)
	{
		case SERIAL_COMPLETE: // in case more than 1 command is sent at a time
		case SERIAL_PREFIX:
			switch(input)
			{
				case 'L':
					instance.state = SERIAL_LANE;
					break;
				case 'R': // Reset/clear all configuration

					// disable interrupts on latch/clock/data for now
					HAL_NVIC_DisableIRQ(EXTI0_IRQn);
					HAL_NVIC_DisableIRQ(EXTI1_IRQn);
					HAL_NVIC_DisableIRQ(EXTI4_IRQn);
					HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

					Disable8msTimer();
					DisableP1ClockTimer();
					DisableP2ClockTimer();

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

					// important to reset our state
					recentLatch = 0;
					toggleNext = 0;
					p1_current_bit = 0;
					p2_current_bit = 0;
					dpcmFix = 0;
					clockFix = 0;
					request_pending = 0;
					bulk_mode = 0;

					memset((uint32_t*)&P1_GPIOC_current, 0, 128);
					memset((uint32_t*)&P1_GPIOC_next, 0, 128);
					memset((uint32_t*)&P2_GPIOC_current, 0, 128);
					memset((uint32_t*)&P2_GPIOC_next, 0, 128);

					memset((uint32_t*)&V1_GPIOB_current, 0, 64);
					memset((uint32_t*)&V1_GPIOB_next, 0, 64);
					memset((uint32_t*)&V2_GPIOC_current, 0, 64);
					memset((uint32_t*)&V2_GPIOC_next, 0, 64);

					ResetTASRuns();
					serial_interface_output((uint8_t*)"\x01R", 2); // good response for reset
					instance.state = SERIAL_COMPLETE;
					break;
				case 'A': // Run #1 controller data
					instance.selected_run = RUN_A;
					instance.state = SERIAL_CONTROLLER_DATA_START;
					break;
				case 'a': // 28 frame data burst is complete
					request_pending = 0;
					break;
				case 'Q':
					instance.state = SERIAL_CMD_Q_1;
					break;
				case 'S': // Setup a run
					instance.state = SERIAL_SETUP;
					break;
				case 'T': // Transition
					instance.state = SERIAL_TRANSITION;
					break;
				case 'P': // Power controls
					instance.state = SERIAL_POWER;
					break;
				case '\xDF':
					jumpToDFU = 1;
					break;
				default: // Error: prefix not understood
					serial_interface_output((uint8_t*)"\xFF", 1);
					break;
			}
			break;

		case SERIAL_CMD_Q_1:
			if(input != 'A')
			{
				serial_interface_output((uint8_t*)"\xFE", 1); // run not supported
				instance.selected_run = RUN_NONE;
				instance.state = SERIAL_COMPLETE;
			} else {
				instance.state = SERIAL_CMD_Q_2;
			}
			break;
		case SERIAL_CMD_Q_2:
			if (input == '1') // enter bulk transfer mode
			{
				bulk_mode = 1;
			} else if (input == '0') // exit bulk transfer mode
			{
				bulk_mode = 0;
			} else // should not reach this
			{
				serial_interface_output((uint8_t*) "\xFA", 1); // Error during bulk transfer toggle
				instance.selected_run = RUN_NONE;
			}
			instance.state = SERIAL_COMPLETE;
			break;


		case SERIAL_LANE:
			if(input == 'A')
			{
				EXTI1_IRQHandler(); // simulate that a latch has occurred
			}

			instance.state = SERIAL_COMPLETE;
			break;
		case SERIAL_POWER:
			switch(input)
			{
				case '0': // power off
					GPIOA->BSRR = (1 << SNES_RESET_LOW_A);
					break;
				case '1': // power on
					GPIOA->BSRR = (1 << SNES_RESET_HIGH_A);
					break;
				case 'S': // soft reset
					GPIOA->BSRR = (1 << SNES_RESET_LOW_A);
					HAL_Delay(200);
					GPIOA->BSRR = (1 << SNES_RESET_HIGH_A);
					HAL_Delay(200);
					break;
				case 'H': // hard reset
					GPIOA->BSRR = (1 << SNES_RESET_LOW_A);
					HAL_Delay(1000);
					GPIOA->BSRR = (1 << SNES_RESET_HIGH_A);
					HAL_Delay(1000);
					break;
				default:
					// TODO: should this emit an error for unknown power config?
					break;
			}
			instance.state = SERIAL_COMPLETE;
			break;

		case SERIAL_CONTROLLER_DATA_START:
			instance.controller_data_bytes_read = 0;
			instance.state = SERIAL_CONTROLLER_DATA_CONTINUE;
			// fall through
		case SERIAL_CONTROLLER_DATA_CONTINUE:
			instance.controller_data_buffer[instance.controller_data_bytes_read++] = input;
			if (instance.controller_data_bytes_read < GetSizeOfInputForRun(instance.selected_run))
			{
				// wait for next byte...
				break;
			}

			// Got the full frame so add it to the RunData

			if (ExtractDataAndAddFrame(instance.selected_run, instance.controller_data_buffer, instance.controller_data_bytes_read) == 0)
			{
				// buffer must have been full
				serial_interface_output((uint8_t*)"\xB0", 1);
			}

			if(!TASRunIsInitialized(0) && TASRunGetSize(0) > 0) // this should only run once per run to set up the 1st frame of data
			{

				Console c = TASRunGetConsole(0);
				if(c == CONSOLE_NES || c == CONSOLE_SNES)
				{
					if(TASRunGetDPCMFix(0))
					{
						toggleNext = 1;
					}
					if(TASRunGetClockFix(0))
					{
						clockFix = 1;
					}

					EXTI1_IRQHandler();
				}

				TASRunSetInitialized(0, 1);

				if(c == CONSOLE_NES || c == CONSOLE_SNES)
				{
					HAL_NVIC_EnableIRQ(EXTI0_IRQn);
					HAL_NVIC_EnableIRQ(EXTI1_IRQn);
					HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
				}
				else if(c == CONSOLE_N64 || c == CONSOLE_GC)
				{
					HAL_NVIC_EnableIRQ(EXTI4_IRQn);
				}
			}

			instance.state = SERIAL_COMPLETE;
			instance.selected_run = RUN_NONE;
			break;
		case SERIAL_CONSOLE:
			switch(input)
			{
				case 'M': // setup N64
					TASRunSetConsole(instance.selected_run, CONSOLE_N64);
					SetN64Mode();
					instance.state = SERIAL_NUM_CONTROLLERS;
					break;
				case 'G': // setup Gamecube
					TASRunSetConsole(instance.selected_run, CONSOLE_GC);
					SetN64Mode();
					instance.state = SERIAL_NUM_CONTROLLERS;
					break;
				case 'S': // setup SNES
					TASRunSetConsole(instance.selected_run, CONSOLE_SNES);
					SetSNESMode();
					instance.state = SERIAL_NUM_CONTROLLERS;
					break;
				case 'N': // setup NES
					TASRunSetConsole(instance.selected_run, CONSOLE_NES);
					SetSNESMode();
					instance.state = SERIAL_NUM_CONTROLLERS;
					break;
				default: // Error: console type not understood
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFC", 1);
					break;
			}
			break;
		case SERIAL_SETUP:
			switch(input)
			{
				case 'A': // setup Run #1
					instance.selected_run = RUN_A;
					instance.state = SERIAL_CONSOLE;
					break;
				default: // Error: run number not understood
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFE", 1);
					break;
			}
			break;
		case SERIAL_NUM_CONTROLLERS:
		{
			uint8_t p1 = (input >> 4);
			uint8_t p2 = (input & 0xF);
			uint8_t p1_lanes = 0, p2_lanes = 0;

			if(p1 == 0x8)
				p1_lanes = 1;
			else if(p1 == 0xC)
				p1_lanes = 2;
			else if(p1 == 0xE)
				p1_lanes = 3;

			if(p2 == 0x8)
				p2_lanes = 1;
			else if(p2 == 0xC)
				p2_lanes = 2;
			else if(p2 == 0xE)
				p2_lanes = 3;

			if(p1 != 0) // player 1 better have some kind of data!
			{
				if(p2 != 0) // 2 controllers
				{
					TASRunSetNumControllers(instance.selected_run, 2);

					if(p1_lanes == p2_lanes)
					{
						TASRunSetNumDataLanes(instance.selected_run, p1_lanes);
					}
					else // error
					{
						serial_interface_output((uint8_t*)"\xFD", 1);
					}
				}
				else // 1 controller
				{
					TASRunSetNumControllers(instance.selected_run, 1);
					TASRunSetNumDataLanes(instance.selected_run, p1_lanes);
				}
				instance.state = SERIAL_SETTINGS;
			}
			else
			{
				instance.state = SERIAL_COMPLETE;
				instance.selected_run = RUN_NONE;
				serial_interface_output((uint8_t*)"\xFD", 1);
			}
			break;
		}
		case SERIAL_SETTINGS:
			TASRunSetDPCMFix(0, ((input >> 7) & 1));
			TASRunSetOverread(0, ((input >> 6) & 1));
			// acceptable values for clock fix: 0 --> 63
			// effective range of clock fix timer: 0us --> 15.75 us
			TASRunSetClockFix(0, input & 0x3F); // get lower 6 bits
			ReInitClockTimers();

			serial_interface_output((uint8_t*)"\x01S", 2);

			instance.state = SERIAL_COMPLETE;
			instance.selected_run = RUN_NONE;
			break;
		case SERIAL_TRANSITION:
			// process 2nd character in command for run letter
			if(input == 'A')
			{
				instance.selected_run = RUN_A;
				instance.state = SERIAL_TRANSITION_1;
			}
			else
			{
				instance.state = SERIAL_COMPLETE;
				instance.selected_run = RUN_NONE;
				serial_interface_output((uint8_t*)"\xFE", 1);
			}
			break;
		case SERIAL_TRANSITION_1:
			// ignore 3rd byte
			instance.state = SERIAL_TRANSITION_2;
			break;
		case SERIAL_TRANSITION_2:
			instance.controller_data_bytes_read = 0;
			instance.state = SERIAL_TRANSITION_3;  // intentional fall through
		case SERIAL_TRANSITION_3:
			if (instance.controller_data_bytes_read < sizeof(uint32_t))
			{
				instance.controller_data_buffer[instance.controller_data_bytes_read] = input;
				break;
			}

			uint32_t tempVal;
			memcpy(&tempVal, instance.controller_data_buffer, sizeof(uint32_t));

			if(input == 'A') // transition to ACE
			{
				if(!AddTransition(0, TRANSITION_ACE, tempVal)) // try adding transition
				{
					// adding transition failed
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFB", 1);
					break;
				}
			}
			else if(input == 'N')
			{
				if(!AddTransition(0, TRANSITION_NORMAL, tempVal)) // try adding transition
				{
					// adding transition failed
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFB", 1);
					break;
				}
			}
			else if(input == 'S')
			{
				if(!AddTransition(0, TRANSITION_RESET_SOFT, tempVal)) // try adding transition
				{
					// adding transition failed
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFB", 1);
					break;
				}
			}
			else if(input == 'H')
			{
				if(!AddTransition(0, TRANSITION_RESET_HARD, tempVal)) // try adding transition
				{
					// adding transition failed
					instance.state = SERIAL_COMPLETE;
					instance.selected_run = RUN_NONE;
					serial_interface_output((uint8_t*)"\xFB", 1);
					break;
				}
			}

			instance.state = SERIAL_COMPLETE;
			instance.selected_run = RUN_NONE;
			break;
		default:
			break;
	}

}

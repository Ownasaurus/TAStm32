#include "serial_interface.h"

#include "stm32f4xx_it.h"
#include "main.h"
#include "TASRun.h"

#include <string.h>
#include <stdlib.h>

// only instance of this, but make callers use access functions
static serial_interface_state_t instance;

static uint8_t NullOutputFunction(uint8_t *buffer, uint16_t n)
{
	return 0;
}

void serial_interface_reset() {
	instance.state = SERIAL_COMPLETE;
	serial_interface_set_output_function(NullOutputFunction);
}

void serial_interface_set_output_function(OutputFunction func)
{
	instance.output_func = func;
}

inline uint8_t serial_interface_output(uint8_t *buffer, uint16_t n)
{
	return instance.output_func(buffer, n);
}

void serial_interface_consume(uint8_t *buffer, uint32_t n)
{
	for (uint32_t i = 0; i != n; ++i)
	{
		uint8_t input = buffer[i];
		switch(instance.state)
		{
			case SERIAL_COMPLETE: // in case more than 1 command is sent at a time
			case SERIAL_PREFIX:
				switch(input)
				{
					case 'U': // set up latch train for a run
						instance.state = SERIAL_TRAIN_RUN;
						break;
					case 'L':
						instance.state = SERIAL_LANE;
						break;
					case 'R': // Reset/clear all configuration

						ResetRun();
						serial_interface_output((uint8_t*)"\x01R", 2); // good response for reset
						instance.state = SERIAL_COMPLETE;
						break;
					case 'A': // Run #1 controller data
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
			case SERIAL_TRAIN_RUN:
				if(input != 'A')
				{
					serial_interface_output((uint8_t*)"\xFE", 1); // run not supported
					instance.state = SERIAL_COMPLETE;
				}
				else
				{
					trains_enabled = 1;
					instance.latch_train_index = 0;
					instance.state = SERIAL_TRAIN_LEN_1;
				}
				break;
			case SERIAL_TRAIN_LEN_1:
				instance.latch_train_length = input;
				instance.state = SERIAL_TRAIN_LEN_2;
				break;
			case SERIAL_TRAIN_LEN_2:
				instance.latch_train_length += (input << 8);
				instance.state = SERIAL_TRAIN_VAL_1;

				latch_trains = (uint16_t*)malloc(sizeof(uint16_t)*instance.latch_train_length);
				break;
			case SERIAL_TRAIN_VAL_1:
				latch_trains[instance.latch_train_index] = input; // put low 8 bits
				instance.state = SERIAL_TRAIN_VAL_2;
				break;
			case SERIAL_TRAIN_VAL_2:
				latch_trains[instance.latch_train_index++] += (input << 8); // put high 8 bits and advance index

				if(instance.latch_train_index >= instance.latch_train_length) // done with latch train
				{
					instance.state = SERIAL_COMPLETE;
				}
				else
				{
					instance.state = SERIAL_TRAIN_VAL_1;
				}
				break;
			case SERIAL_CMD_Q_1:
				if(input != 'A')
				{
					serial_interface_output((uint8_t*)"\xFE", 1); // run not supported
					instance.state = SERIAL_COMPLETE;
				}
				else
				{
					instance.state = SERIAL_CMD_Q_2;
				}
				break;
			case SERIAL_CMD_Q_2:
				if (input == '1') // enter bulk transfer mode
				{
					bulk_mode = 1;
				}
				else if (input == '0') // exit bulk transfer mode
				{
					bulk_mode = 0;
				}
				else // should not reach this
				{
					serial_interface_output((uint8_t*) "\xFA", 1); // Error during bulk transfer toggle
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
				if (instance.controller_data_bytes_read < tasrun->input_data_size)
				{
					// wait for next byte...
					break;
				}

				// Got the full frame so add it to the RunData

				if (ExtractDataAndAddFrame(instance.controller_data_buffer, instance.controller_data_bytes_read) == 0)
				{
					// buffer must have been full
					serial_interface_output((uint8_t*)"\xB0", 1);
				}

				if(!tasrun->initialized && tasrun->size > 0) // this should only run once per run to set up the 1st frame of data
				{

					Console c = tasrun->console;
					if(c == CONSOLE_NES || c == CONSOLE_SNES)
					{
						if(tasrun->dpcmFix)
						{
							toggleNext = 1;
						}
						if(TASRunGetClockFix())
						{
							clockFix = 1;
						}

						EXTI1_IRQHandler();
					}

					tasrun->initialized = 1;

					if(c == CONSOLE_NES || c == CONSOLE_SNES)
					{
						// enable these interrupts atomically
						__disable_irq();
						HAL_NVIC_EnableIRQ(EXTI0_IRQn);
						HAL_NVIC_EnableIRQ(EXTI1_IRQn);
						HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
						if (tasrun->multitap)
							HAL_NVIC_EnableIRQ(EXTI4_IRQn);
						__enable_irq();
					}
					else if(c == CONSOLE_N64 || c == CONSOLE_GC)
					{
						HAL_NVIC_EnableIRQ(EXTI4_IRQn);
					}
				}

				instance.state = SERIAL_COMPLETE;
				break;
			case SERIAL_CONSOLE:
				switch(input)
				{
					case 'M': // setup N64
						TASRunSetConsole(CONSOLE_N64);
						SetN64Mode();
						instance.state = SERIAL_NUM_CONTROLLERS;
						break;
					case 'G': // setup Gamecube
						TASRunSetConsole(CONSOLE_GC);
						SetN64Mode();
						instance.state = SERIAL_NUM_CONTROLLERS;
						break;
					case 'S': // setup SNES
						TASRunSetConsole(CONSOLE_SNES);
						SetSNESMode();
						instance.state = SERIAL_NUM_CONTROLLERS;
						break;
					case 'N': // setup NES
						TASRunSetConsole(CONSOLE_NES);
						SetSNESMode();
						instance.state = SERIAL_NUM_CONTROLLERS;
						break;
					default: // Error: console type not understood
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFC", 1);
						break;
				}
				break;
			case SERIAL_SETUP:
				switch(input)
				{
					case 'A': // setup Run #1
						instance.state = SERIAL_CONSOLE;
						break;
					default: // Error: run number not understood
						instance.state = SERIAL_COMPLETE;
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
				else if(p1 == 0xF){
					p1_lanes = 4;
					if (tasrun->console == CONSOLE_SNES)
						tasrun->multitap = 1;
				}

				if(p2 == 0x8)
					p2_lanes = 1;
				else if(p2 == 0xC)
					p2_lanes = 2;
				else if(p2 == 0xE)
					p2_lanes = 3;
				else if(p2 == 0xF){
					p2_lanes = 4;
					if (tasrun->console == CONSOLE_SNES)
						tasrun->multitap = 1;
				}

				if (tasrun->multitap)
					SetMultitapMode();

				if(p1 != 0) // player 1 better have some kind of data!
				{
					if(p2 != 0) // 2 controllers
					{
						TASRunSetNumControllers(2);

						if(p1_lanes == p2_lanes)
						{
							TASRunSetNumDataLanes(p1_lanes);
						}
						else // error
						{
							serial_interface_output((uint8_t*)"\xFD", 1);
						}
					}
					else // 1 controller
					{
						TASRunSetNumControllers(1);
						TASRunSetNumDataLanes(p1_lanes);
					}
					instance.state = SERIAL_SETTINGS;
				}
				else
				{
					instance.state = SERIAL_COMPLETE;
					serial_interface_output((uint8_t*)"\xFD", 1);
				}
				break;
			}
			case SERIAL_SETTINGS:
				tasrun->dpcmFix = (((input >> 7) & 1));
				tasrun->overread = (((input >> 6) & 1));
				// acceptable values for clock fix: 0 --> 63
				// effective range of clock fix timer: 0us --> 15.75 us
				TASRunSetClockFix(input & 0x3F); // get lower 6 bits
				ReInitClockTimers();

				serial_interface_output((uint8_t*)"\x01S", 2);

				instance.state = SERIAL_COMPLETE;
				break;
			case SERIAL_TRANSITION:
				// process 2nd character in command for run letter
				if(input == 'A')
				{
					instance.state = SERIAL_TRANSITION_1;
				}
				else
				{
					instance.state = SERIAL_COMPLETE;
					serial_interface_output((uint8_t*)"\xFE", 1);
				}
				break;
			case SERIAL_TRANSITION_1:
				// 3rd byte is transition type
				instance.transition_type = input;
				instance.state = SERIAL_TRANSITION_2;
				break;
			case SERIAL_TRANSITION_2:
				// next 4 bytes are transition frame number (uint32_t)
				instance.controller_data_bytes_read = 0;
				instance.state = SERIAL_TRANSITION_3;  // intentional fall through
			case SERIAL_TRANSITION_3:
				instance.controller_data_buffer[instance.controller_data_bytes_read++] = input;

				if(instance.controller_data_bytes_read < sizeof(uint32_t)) // only save 4 bytes
				{
					break;
				}

				uint32_t tempVal;
				memcpy(&tempVal, instance.controller_data_buffer, sizeof(tempVal));

				// now make a decision based off of the 3rd byte noted earlier
				if(instance.transition_type == 'A') // transition to ACE
				{
					if(!AddTransition(TRANSITION_ACE, tempVal)) // try adding transition
					{
						// adding transition failed
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(instance.transition_type == 'N')
				{
					if(!AddTransition(TRANSITION_NORMAL, tempVal)) // try adding transition
					{
						// adding transition failed
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(instance.transition_type == 'S')
				{
					if(!AddTransition(TRANSITION_RESET_SOFT, tempVal)) // try adding transition
					{
						// adding transition failed
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(instance.transition_type == 'H')
				{
					if(!AddTransition(TRANSITION_RESET_HARD, tempVal)) // try adding transition
					{
						// adding transition failed
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(instance.transition_type == 'W')
				{
					if(!AddTransition(TRANSITION_WAIT_AUDIO, tempVal)) // try adding transition
					{
						// adding transition failed
						instance.state = SERIAL_COMPLETE;
						serial_interface_output((uint8_t*)"\xFB", 1);
						break;
					}
				}

				instance.state = SERIAL_COMPLETE;
				break;
			default:
				break;
		}
	}

}

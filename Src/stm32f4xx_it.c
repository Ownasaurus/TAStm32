/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "n64.h"
#include "TASRun.h"
#include "usbd_cdc_if.h"
#include "serial_interface.h"
#include "usbplayback/inputs.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const uint8_t P1_D0_HIGH_C = 3;
const uint8_t P1_D0_LOW_C = 19;
const uint8_t P1_D1_HIGH_C = 2;
const uint8_t P1_D1_LOW_C = 18;
const uint8_t P1_D2_HIGH_C = 4;
const uint8_t P1_D2_LOW_C = 20;
const uint8_t P2_D0_HIGH_C = 8;
const uint8_t P2_D0_LOW_C = 24;
const uint8_t P2_D1_HIGH_C = 7;
const uint8_t P2_D1_LOW_C = 23;
const uint8_t P2_D2_HIGH_C = 9;
const uint8_t P2_D2_LOW_C = 25;
const uint8_t SNES_RESET_HIGH_A = 9;
const uint8_t SNES_RESET_LOW_A = 25;

const uint8_t V1_D0_HIGH_B = 7;
const uint8_t V1_D0_LOW_B = 23;
const uint8_t V1_D1_HIGH_B = 6;
const uint8_t V1_D1_LOW_B = 22;
const uint8_t V1_LATCH_HIGH_B = 5;
const uint8_t V1_LATCH_LOW_B = 21;
const uint8_t V1_CLOCK_HIGH_B = 4;
const uint8_t V1_CLOCK_LOW_B = 20;

const uint8_t V2_D0_HIGH_C = 12;
const uint8_t V2_D0_LOW_C = 28;
const uint8_t V2_D1_HIGH_C = 11;
const uint8_t V2_D1_LOW_C = 27;
const uint8_t V2_LATCH_HIGH_C = 10;
const uint8_t V2_LATCH_LOW_C = 26;
const uint8_t V2_CLOCK_HIGH_A = 15;
const uint8_t V2_CLOCK_LOW_A = 31;

const uint32_t P1_D0_MASK = 0x00080008;
const uint32_t P1_D1_MASK = 0x00040004;
const uint32_t P1_D2_MASK = 0x00100010;
const uint32_t P2_D0_MASK = 0x01000100;
const uint32_t P2_D1_MASK = 0x00800080;
const uint32_t P2_D2_MASK = 0x02000200;

const uint32_t ALL_MASK = 0x039C039C;

RunDataArray *dataptr = 0;

#define MODER_DATA_MASK 0xFFF03C0F

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define WAIT_4_CYCLES __asm("ADD     R1, R1, #0\nADD     R1, R1, #0\nADD     R1, R1, #0\nADD     R1, R1, #0")
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
// variables that could change due to higher-priority interrupts
volatile uint8_t p1_clock_filtered;
volatile uint8_t p2_clock_filtered;

volatile uint8_t recentLatch;

GENControllerData gen_blank = {0};

// leave enough room for SNES + overread
uint32_t P1_GPIOC_current[17];
uint32_t P1_GPIOC_next[17];

uint32_t P2_GPIOC_current[17];
uint32_t P2_GPIOC_next[17];

// extra words for multitap

uint32_t P1_GPIOC_current_multitap[17];
uint32_t P1_GPIOC_next_multitap[17];

uint32_t P2_GPIOC_current_multitap[17];
uint32_t P2_GPIOC_next_multitap[17];

// leave enough room for SNES only
uint32_t V1_GPIOB_current[16];
uint32_t V1_GPIOB_next[16];

uint32_t V2_GPIOC_current[16];
uint32_t V2_GPIOC_next[16];

uint8_t p1_current_bit;
uint8_t p2_current_bit;

uint8_t toggleNext;
uint8_t dpcmFix;
uint8_t clockFix;

uint16_t p1_d0_next;
uint16_t p1_d1_next;
uint16_t p2_d0_next;
uint16_t p2_d1_next;

uint16_t p1_d1_next_multitap;
uint16_t p2_d0_next_multitap;
uint16_t p1_d0_next_multitap;
uint16_t p2_d1_next_multitap;

uint8_t request_pending = 0;
uint8_t bulk_mode = 0;

// latch train vars
uint16_t current_train_index;
uint16_t current_train_latch_count;
uint8_t between_trains = 1;
uint8_t trains_enabled;
uint8_t firstLatch = 0;

uint16_t* latch_trains;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void my_wait_us_asm(int n);
static uint8_t UART2_OutputFunction(uint8_t *buffer, uint16_t n);
static HAL_StatusTypeDef Simple_Transmit(UART_HandleTypeDef *huart);
void GCN64_CommandStart(uint8_t player);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t multitapSel = 1;
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim10;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles EXTI line 0 interrupt.
  */
void EXTI0_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI0_IRQn 0 */
	// P1_CLOCK

	if(!p1_clock_filtered)
	{
		if(clockFix)
		{
			my_wait_us_asm(2); // necessary to prevent switching too fast in DPCM fix mode
		}

		uint32_t p1_data = multitapSel ? P1_GPIOC_current[p1_current_bit] : P1_GPIOC_current_multitap[p1_current_bit];
		GPIOC->BSRR = p1_data;

		ResetAndEnableP1ClockTimer();
		if(p1_current_bit < 16)
		{
			p1_current_bit++;
		}
	}
  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line 1 interrupt.
  */
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */

	// P1_LATCH
	int8_t regbit = 50, databit = -1; // random initial values

	// set relevant data ports as output if this is the first latch
	if(firstLatch && (EXTI->PR & P1_LATCH_Pin))
	{
		GPIOC->MODER = (GPIOC->MODER & MODER_DATA_MASK) | tasrun->moder_firstLatch;
		firstLatch = 0;
	}

	if(tasrun->console == CONSOLE_GEN)
	{
		// comment format below: [PIN1 PIN2 PIN3 PIN4 PIN5 PIN6 PIN7 PIN8 PIN9]
		// which translates to.. [P1D0 P1D1 P1D2 P2D0 5V   P2D1 SEL  GND  P2D2]
		// whose colors are..    [BRN  ORG  GREY BLK  RED  YEL  BLU  WHI  GRN ]
		// LOW means pressed, so we put a 1 there
		// HIGH means un-pressed, so we put a 0 there

		GENControllerData* pData = (GENControllerData*)dataptr;
		if(!pData)
		{
			pData = &gen_blank;
		}

		if(P1_LATCH_GPIO_Port->IDR & P1_LATCH_Pin) // rising edge
		{
			// [U D L R B C]
			P1_GPIOC_next[0] = 	(pData->up << P1_D0_LOW_C) | (pData->down << P1_D1_LOW_C) | (pData->left << P1_D2_LOW_C) |
					(pData->right << P2_D0_LOW_C) | (pData->b << P2_D1_LOW_C) | (pData->c << P2_D2_LOW_C);
			P1_GPIOC_next[0] |= (((~P1_GPIOC_next[0]) & (ALL_MASK)) >> 16);

			GPIOC->BSRR = P1_GPIOC_next[0];

			dataptr = GetNextFrame();
			serial_interface_output((uint8_t*)"A", 1); // tell python that we processed a frame
		}
		else // falling edge
		{
			// [U D LOW LOW A Start]
			P1_GPIOC_next[0] = 	(pData->up << P1_D0_LOW_C) | (pData->down << P1_D1_LOW_C) | (1 << P1_D2_LOW_C) |
					(1 << P2_D0_LOW_C) | (pData->a << P2_D1_LOW_C) | (pData->start << P2_D2_LOW_C);
			P1_GPIOC_next[0] |= (((~P1_GPIOC_next[0]) & (ALL_MASK)) >> 16);

			GPIOC->BSRR = P1_GPIOC_next[0];
		}
	}
	else
	{
		if(recentLatch == 0) // no recent latch
		{
			// quickly set first bit of data for the next frame
			uint32_t p1_data = P1_GPIOC_next[0];
			uint32_t p2_data = P2_GPIOC_next[0];
			uint32_t all_data = (p1_data | p2_data);
			GPIOC->BSRR = all_data;

			// copy the 2nd bit over too
			__disable_irq();
			P1_GPIOC_current[1] = P1_GPIOC_next[1];
			P2_GPIOC_current[1] = P2_GPIOC_next[1];
			p1_current_bit = p2_current_bit = 1; // set the next bit to be read
			__enable_irq();

			// copy the rest of the bits. do not copy the overread since it will never change
			// P2 comes before P1 in NES, so copy P2 first
			memcpy(P2_GPIOC_current, P2_GPIOC_next, 64);
			memcpy(P1_GPIOC_current, P1_GPIOC_next, 64);

			// copy the multitap bits too
			if (tasrun->multitap) {
				memcpy(P2_GPIOC_current_multitap, P2_GPIOC_next_multitap, 64);
				memcpy(P1_GPIOC_current_multitap, P1_GPIOC_next_multitap, 64);
				// Assume sel is high, this should perhaps be being being reset by the EXT4 interrupt if it was set to trigger both edges
				multitapSel = 1;
			}

			// now prepare for the next frame!

			if(toggleNext == 1)
			{
				dpcmFix = 1 - dpcmFix;
			}
			else if(toggleNext == 2)
			{
				GPIOA->BSRR = (1 << SNES_RESET_LOW_A);
				HAL_Delay(200);
				GPIOA->BSRR = (1 << SNES_RESET_HIGH_A);
				HAL_Delay(200);
			}
			else if(toggleNext == 3)
			{
				GPIOA->BSRR = (1 << SNES_RESET_LOW_A);
				HAL_Delay(1000);
				GPIOA->BSRR = (1 << SNES_RESET_HIGH_A);
				HAL_Delay(1000);
			}

			if(dpcmFix)
			{
				recentLatch = 1; // repeat input on latch
				ResetAndEnable8msTimer(); // start timer and proceed as normal
			}

			if(trains_enabled)
			{
				if(between_trains == 1) // at least one lag frame detected
				{
					// do what you gotta do
					// adjust the frame of the run accordingly
					int diff = latch_trains[current_train_index] - current_train_latch_count;

					if(diff == 1) // we are one latch short
					{
						GetNextFrame(); // burn a frame of data
						dataptr = GetNextFrame(); // use this frame instead
						serial_interface_output((uint8_t*)"UB", 2);
					}
					else if(diff == -1) // we had one extra latch
					{
						// do NOT get next frame (yet). hold back for one
						serial_interface_output((uint8_t*)"UA", 2);
					}
					else if(diff != 0) // large deviation
					{
						// AHHHH!!!!!! Give some sort of unrecoverable error?
						serial_interface_output((uint8_t*)"UF", 2);
					}
					else // normalcy
					{
						dataptr = GetNextFrame();
						serial_interface_output((uint8_t*)"UC", 2);
					}

					current_train_index++; // we have begun the next train
					current_train_latch_count = 1; // reset the latch count
					between_trains = 0; // we are no longer between trains
				}
				else
				{
					current_train_latch_count++;
					dataptr = GetNextFrame();
				}

				DisableTrainTimer(); // reset counters back to 0
				ResetAndEnableTrainTimer();
			}
			else
			{
				dataptr = GetNextFrame();
			}

			if(dataptr)
			{
				toggleNext = TASRunIncrementFrameCount();

				databit = 0;
				if(tasrun->console == CONSOLE_NES)
				{
					databit = 7; // number of bits of NES - 1

					memcpy((uint8_t*)&p1_d0_next, &dataptr[0][0][0], sizeof(NESControllerData));
					memcpy((uint8_t*)&p1_d1_next, &dataptr[0][0][1], sizeof(NESControllerData));
					memcpy((uint8_t*)&p2_d0_next, &dataptr[0][1][0], sizeof(NESControllerData));
					memcpy((uint8_t*)&p2_d1_next, &dataptr[0][1][1], sizeof(NESControllerData));
				}
				else
				{
					databit = 15; // number of bits of SNES - 1

					memcpy((uint16_t*)&p1_d0_next, &dataptr[0][0][0], sizeof(SNESControllerData));
					memcpy((uint16_t*)&p1_d1_next, &dataptr[0][0][1], sizeof(SNESControllerData));
					memcpy((uint16_t*)&p2_d0_next, &dataptr[0][1][0], sizeof(SNESControllerData));
					memcpy((uint16_t*)&p2_d1_next, &dataptr[0][1][1], sizeof(SNESControllerData));

					// fix endianness
					p1_d0_next = ((p1_d0_next >> 8) & 0xFF) | ((p1_d0_next << 8) & 0xFF00);
					p1_d1_next = ((p1_d1_next >> 8) & 0xFF) | ((p1_d1_next << 8) & 0xFF00);
					p2_d0_next = ((p2_d0_next >> 8) & 0xFF) | ((p2_d0_next << 8) & 0xFF00);
					p2_d1_next = ((p2_d1_next >> 8) & 0xFF) | ((p2_d1_next << 8) & 0xFF00);

					// data lines 2 & 3 are multitap d0 and d1
					if (tasrun->multitap) {
						memcpy((uint16_t*) &p1_d0_next_multitap, &dataptr[0][0][2], sizeof(SNESControllerData));
						memcpy((uint16_t*) &p1_d1_next_multitap, &dataptr[0][0][3], sizeof(SNESControllerData));
						memcpy((uint16_t*) &p2_d0_next_multitap, &dataptr[0][1][2], sizeof(SNESControllerData));
						memcpy((uint16_t*) &p2_d1_next_multitap, &dataptr[0][1][3], sizeof(SNESControllerData));

						// fix endianness
						p1_d0_next_multitap = ((p1_d0_next_multitap >> 8) & 0xFF) | ((p1_d0_next_multitap << 8) & 0xFF00);
						p1_d1_next_multitap = ((p1_d1_next_multitap >> 8) & 0xFF) | ((p1_d1_next_multitap << 8) & 0xFF00);
						p2_d0_next_multitap = ((p2_d0_next_multitap >> 8) & 0xFF) | ((p2_d0_next_multitap << 8) & 0xFF00);
						p2_d1_next_multitap = ((p2_d1_next_multitap >> 8) & 0xFF) | ((p2_d1_next_multitap << 8) & 0xFF00);

					}
				}

				regbit = 0;

				// fill the regular data
				while(databit >= 0)
				{
					uint32_t temp;
					temp = 					(uint32_t)(((p1_d0_next >> databit) & 1) << P1_D0_LOW_C) |
											(uint32_t)(((p1_d1_next >> databit) & 1) << P1_D1_LOW_C);
					P1_GPIOC_next[regbit] = temp | (((~temp) & (P1_D0_MASK | P1_D1_MASK)) >> 16);

					temp = 					(uint32_t)(((p2_d0_next >> databit) & 1) << P2_D0_LOW_C) |
											(uint32_t)(((p2_d1_next >> databit) & 1) << P2_D1_LOW_C);
					P2_GPIOC_next[regbit] = temp | (((~temp) & (P2_D0_MASK | P2_D1_MASK)) >> 16);

					temp = 					(uint32_t)(((p1_d0_next >> databit) & 1) << V1_D0_HIGH_B) |
											(uint32_t)(((p1_d1_next >> databit) & 1) << V1_D1_HIGH_B);
					V1_GPIOB_next[regbit] = temp | (((~temp) & 0x00C0) << 16);

					temp = 					(uint32_t)(((p2_d0_next >> databit) & 1) << V2_D0_HIGH_C) |
											(uint32_t)(((p2_d1_next >> databit) & 1) << V2_D1_HIGH_C);
					V2_GPIOC_next[regbit] = temp | (((~temp) & 0x1800) << 16);

					regbit++;
					databit--;
				}
				// fill multitap data
				if (tasrun->multitap) {
					regbit = 0;
					databit = 15;
					while (databit >= 0) {
						uint32_t temp;
						temp = (uint32_t) (((p1_d0_next_multitap >> databit) & 1) << P1_D0_LOW_C) | (uint32_t) (((p1_d1_next_multitap >> databit) & 1) << P1_D1_LOW_C);
						P1_GPIOC_next_multitap[regbit] = temp | (((~temp) & (P1_D0_MASK | P1_D1_MASK)) >> 16);

						temp = (uint32_t) (((p2_d0_next_multitap >> databit) & 1) << P2_D0_LOW_C) | (uint32_t) (((p2_d1_next_multitap >> databit) & 1) << P2_D1_LOW_C);
						P2_GPIOC_next_multitap[regbit] = temp | (((~temp) & (P2_D0_MASK | P2_D1_MASK)) >> 16);

						regbit++;
						databit--;
					}
				}
			}
			else // no data left in the buffer
			{
				if(tasrun->console == CONSOLE_NES)
				{
					databit = 7; // number of bits of NES - 1
				}
				else
				{
					databit = 15; // number of bits of SNES - 1
				}

				// no controller data means all SNES pins get set high for this protocol to indicate no buttons are pressed
				// vis pins get all set low to indicate no buttons are pressed
				for(uint8_t index = 0;index <= databit;index++)
				{
					P1_GPIOC_next[index] = (1 << P1_D0_HIGH_C) | (1 << P1_D1_HIGH_C);
					P2_GPIOC_next[index] = (1 << P2_D0_HIGH_C) | (1 << P2_D1_HIGH_C);

					V1_GPIOB_next[index] = (1 << V1_D0_LOW_B) | (1 << V1_D1_LOW_B);
					V2_GPIOC_next[index] = (1 << V2_D0_LOW_C) | (1 << V2_D1_LOW_C);
				}
			}

			if(tasrun->initialized)
			{
				if(bulk_mode)
				{
					if(!request_pending && tasrun->size <= (MAX_SIZE-28)) // not full enough
					{
						if(serial_interface_output((uint8_t*)"a", 1) == USBD_OK) // notify that we latched and want more
						{
							request_pending = 1;
						}
					}
				}
				else
				{
					serial_interface_output((uint8_t*)"A", 1); // notify that we latched
				}
			}
			else
			{
				if(tasrun->console == CONSOLE_NES)
					regbit = 8;
				else
					regbit = 16;

				// fill the overread
				if(tasrun->overread) // overread is 1/HIGH
				{
					// so set logical LOW (NES/SNES button pressed)
					for(uint8_t index = regbit;index < 17;index++)
					{
						P1_GPIOC_current[index] = P1_GPIOC_next[index] = (1 << P1_D0_LOW_C) | (1 << P1_D1_LOW_C);
						P2_GPIOC_current[index] = P2_GPIOC_next[index] = (1 << P2_D0_LOW_C) | (1 << P2_D1_LOW_C);
					}
				}
				else
				{
					for(uint8_t index = regbit;index < 17;index++)
					{
						P1_GPIOC_current[index] = P1_GPIOC_next[index] = (1 << P1_D0_HIGH_C) | (1 << P1_D1_HIGH_C);
						P2_GPIOC_current[index] = P2_GPIOC_next[index] = (1 << P2_D0_HIGH_C) | (1 << P2_D1_HIGH_C);
					}
				}
			}

			// vis board code = 16 clock pulses followed by a latch pulse
			memcpy(V1_GPIOB_current, V1_GPIOB_next, 64);
			memcpy(V2_GPIOC_current, V2_GPIOC_next, 64);
			UpdateVisBoards();
		}
		else if(recentLatch == 1) // multiple close latches and DPCM fix is enabled
		{
			__disable_irq();
			// repeat the same frame of input
			uint32_t p1_data = P1_GPIOC_current[0];
			uint32_t p2_data = P2_GPIOC_current[0];
			uint32_t all_data = (p1_data | p2_data);
			GPIOC->BSRR = all_data;

			p1_current_bit = p2_current_bit = 1;
			__enable_irq();

			ResetAndEnableTrainTimer();
		}
	}

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line 4 interrupt.
  */
void EXTI4_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI4_IRQn 0 */
	// P1_DATA_2 == N64_DATA
	// Read 64 command

	// If this is a SNES run, this means SEL is going LOW so tell clock interrupts
	// to start using the second words of multitap frame
	if (tasrun->console == CONSOLE_SNES)
	{
		if (tasrun->multitap)
		{
			multitapSel = 0;
			p1_current_bit = p2_current_bit = 1;

			// quickly set first bit of data for the next frame
			uint32_t p1_data = P1_GPIOC_current_multitap[0];
			uint32_t p2_data = P2_GPIOC_current_multitap[0];
			uint32_t all_data = (p1_data | p2_data);
			GPIOC->BSRR = all_data;
		}
	}

	// Otherwise process as N64 command
	else
	{
		GCN64_CommandStart(1);
	}

  /* USER CODE END EXTI4_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
  /* USER CODE BEGIN EXTI4_IRQn 1 */

  /* USER CODE END EXTI4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
	Console c = tasrun->console;

	if(c == CONSOLE_N64 || c == CONSOLE_GC)
	{
		GCN64_CommandStart(2);
	}
	else if(c == CONSOLE_SNES || c == CONSOLE_NES)
	{
		// P2_CLOCK
		if(!p2_clock_filtered)
		{
			if(clockFix)
			{
				my_wait_us_asm(2); // necessary to prevent switching too fast in DPCM fix mode
			}

			uint32_t p2_data = multitapSel ? P2_GPIOC_current[p2_current_bit] : P2_GPIOC_current_multitap[p2_current_bit];
			GPIOC->BSRR = p2_data;

			ResetAndEnableP2ClockTimer();
			if(p2_current_bit < 16)
			{
				p2_current_bit++;
			}
		}

	}

	/* USER CODE END EXTI9_5_IRQn 0 */
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_5);
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9); // P2D2
	/* USER CODE BEGIN EXTI9_5_IRQn 1 */
	/* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
  */
void TIM1_UP_TIM10_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 0 */
  between_trains = 1; // if the timer expired, there was at least 20ms between latches. therefore we are between trains.
  DisableTrainTimer(); // to ensure it was a 1-shot

  /* USER CODE END TIM1_UP_TIM10_IRQn 0 */
  HAL_TIM_IRQHandler(&htim10);
  /* USER CODE BEGIN TIM1_UP_TIM10_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM10_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
	inputProcess();
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
  // This is a latch timer
  recentLatch = 0;
  Disable8msTimer(); // to ensure it was a 1-shot

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	uint32_t isrflags   = READ_REG(huart2.Instance->SR);
	uint32_t cr1its     = READ_REG(huart2.Instance->CR1);
	/* UART in mode Transmitter ------------------------------------------------*/
	if (((isrflags & USART_SR_TXE) != RESET) && ((cr1its & USART_CR1_TXEIE) != RESET))
	{
		Simple_Transmit(&huart2);
		return;
	}

	/* UART in mode Transmitter end --------------------------------------------*/
	if (((isrflags & USART_SR_TC) != RESET) && ((cr1its & USART_CR1_TCIE) != RESET))
	{
		/* Disable the UART Transmit Complete Interrupt */
		__HAL_UART_DISABLE_IT(&huart2, UART_IT_TC);

		/* Tx process is ended, restore huart->gState to Ready */
		huart2.gState = HAL_UART_STATE_READY;
		return;
	}

	if(((isrflags & USART_SR_RXNE) != RESET) && ((cr1its & USART_CR1_RXNEIE) != RESET))
	{
		// PROCESS USART2 Rx IRQ HERE
		uint8_t input = ((huart2.Instance)->DR) & (uint8_t)0xFF; // get the last byte from the data register

		serial_interface_set_output_function(UART2_OutputFunction);
		serial_interface_consume(&input, 1);
		return;
	}
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt and DAC1, DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
  // This is a variable clock timer for P1
  p1_clock_filtered = 0;
  DisableP1ClockTimer(); // to ensure it was a 1-shot
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */
  // This is a variable clock timer for P2
  p2_clock_filtered = 0;
  DisableP2ClockTimer(); // to ensure it was a 1-shot
  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go FS global interrupt.
  */
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS global interrupt.
  */
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}

/* USER CODE BEGIN 1 */
static HAL_StatusTypeDef Simple_Transmit(UART_HandleTypeDef *huart)
{
  /* Check that a Tx process is ongoing */
  if (huart->gState == HAL_UART_STATE_BUSY_TX)
  {
    huart->Instance->DR = (uint8_t)(*huart->pTxBuffPtr++ & (uint8_t)0x00FF);

    if (--huart->TxXferCount == 0U)
    {
      /* Disable the UART Transmit Complete Interrupt */
      __HAL_UART_DISABLE_IT(huart, UART_IT_TXE);

      /* Enable the UART Transmit Complete Interrupt */
      __HAL_UART_ENABLE_IT(huart, UART_IT_TC);
    }
    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

void DisableTrainTimer()
{
	TIM10->CNT = 0; // reset count
	TIM10->SR = 0; // reset flags

	HAL_TIM_Base_Stop_IT(&htim10);
}

void ResetAndEnableTrainTimer()
{
	HAL_TIM_Base_Start_IT(&htim10);
}

void Disable8msTimer()
{
	TIM3->CNT = 0; // reset count
	TIM3->SR = 0; // reset flags

	HAL_TIM_Base_Stop_IT(&htim3);
}

void ResetAndEnable8msTimer()
{
	HAL_TIM_Base_Start_IT(&htim3);
}

void DisableP1ClockTimer()
{
	TIM6->CNT = 0; // reset count
	TIM6->SR = 0; // reset flags

	HAL_TIM_Base_Stop_IT(&htim6);
}

void ResetAndEnableP1ClockTimer()
{
	if(clockFix == 0)
	{
		return;
	}

	p1_clock_filtered = 1;

	HAL_TIM_Base_Start_IT(&htim6);
}

void DisableP2ClockTimer()
{
	TIM7->CNT = 0; // reset count
	TIM7->SR = 0; // reset flags

	HAL_TIM_Base_Stop_IT(&htim7);
}

void ResetAndEnableP2ClockTimer()
{
	if(clockFix == 0)
	{
		return;
	}

	p2_clock_filtered = 1;

	HAL_TIM_Base_Start_IT(&htim7);
}

void GCN64_CommandStart(uint8_t player)
{
	GCControllerData gc_data;

	__disable_irq();
	uint32_t cmd;
	static RunDataArray *frame = NULL;
	uint8_t bufferUnderflow = 0;

	cmd = GCN64_ReadCommand(player);

	my_wait_us_asm(2); // wait a small amount of time before replying

	//-------- SEND RESPONSE
	SetN64OutputMode(player);

	switch(cmd)
	{
	  case 0x00: // identity
		  if(tasrun->console == CONSOLE_N64)
		  {
			  N64_SendIdentity(player);
		  }
		  else if(tasrun->console == CONSOLE_GC)
		  {
			  GC_SendIdentity(player);
		  }
		  break;
	  case 0xFF: // N64 reset
		  N64_SendIdentity(player);
		  break;
	  case 0x01: // poll for N64 state
		  if(player == tasrun->numControllers) // only advance frame on highest controller poll
		  {
			  frame = GetNextFrame();
		  }

		  if(frame == NULL) // buffer underflow
		  {
			  bufferUnderflow = 1;
			  N64_SendControllerData(player, 0); // send blank controller data
		  }
		  else
		  {
			  N64_SendRunData(player, frame[0][(player-1)][0].n64_data);
		  }
		  break;
	  case 0x41: //gamecube origin call
		  GC_SendOrigin(player);
		  break;
	  case 0x400302:
	  case 0x400300:
	  case 0x400301:

		tasrun->pollNumber++;
		
		// stop waiting if we recieved a rumble
		// (LSB of command indicates rumble state)
		if (tasrun->waiting && (cmd & 1))
		{
			tasrun->waiting = 0;
			tasrun->pollNumber = 1;
		}

		// start waiting if the previous iteration told us to
		if (toggleNext == 4)
		{
			tasrun->waiting = 1;
			toggleNext = 0;
		}

		// Send blank frame if we're waiting on a rumble
	  	if (tasrun->waiting)
		{
			frame = NULL;
		}
		else if(player == tasrun->numControllers)
		{
			frame = GetNextFrame();
			

			// Skip one out of every thousand frames to work around Melee polling bug
			if (tasrun->meleeMitigation && tasrun->pollNumber % 1000 == 1){
				GetNextFrame();
			}
			if (frame == NULL)
			    bufferUnderflow = 1;
		}

		if(frame == NULL) // buffer underflow or waiting
		{
			memset(&gc_data, 0, sizeof(gc_data));

			gc_data.a_x_axis = 128;
			gc_data.a_y_axis = 128;
			gc_data.c_x_axis = 128;
			gc_data.c_y_axis = 128;
			gc_data.beginning_one = 1;

			GC_SendRunData(player, gc_data); // send blank controller data
		}
		else
		{
			toggleNext = TASRunIncrementFrameCount();
			frame[0][(player-1)][0].gc_data.beginning_one = 1;
			GC_SendRunData(player, frame[0][(player-1)][0].gc_data);
		}
		break;
	  case 0x02:
	  case 0x03:
	  default:
		  // we do not process the read and write commands (memory pack)
		  break;
	}
	//-------- DONE SENDING RESPOSE

	SetN64InputMode(player);

	__enable_irq();

	if(player == 1)
	{
		switch(cmd)
		{
			case 0x01: // N64 poll
				UpdateN64VisBoards(frame[0][0][0].n64_data); //TODO: consider P1 vs P2
			case 0x400302: // GC poll
			case 0x400300: // GC poll
			case 0x400301: // GC poll
				if(bulk_mode)
				{
					if(!request_pending && tasrun->size <= (MAX_SIZE-28)) // not full enough
					{
						if(serial_interface_output((uint8_t*)"a", 1) == USBD_OK)
						{
							request_pending = 1;
						}
					}
				}
				else
				{
					if(bufferUnderflow) // there was a buffer underflow
						serial_interface_output((uint8_t*)"A\xB2", 2);
					else
						serial_interface_output((uint8_t*)"A", 1);
				}


			break;
		}
	}
}

inline void UpdateVisBoards()
{
	if(tasrun->console == CONSOLE_NES)
	{
		// first 8 clock pulses at least 10ns in width
		for(int x = 0;x < 8;x++)
		{
			//set vis data
			GPIOB->BSRR = V1_GPIOB_current[x];
			GPIOC->BSRR = V2_GPIOC_current[x];

			// give time to it to register
			WAIT_4_CYCLES;

			GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
			GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
			// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
			WAIT_4_CYCLES;
			GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
			GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
			WAIT_4_CYCLES;
		}

		// set rest of the vis data to 0s
		GPIOB->BSRR = (1 << V1_D0_LOW_B) | (1 << V1_D1_LOW_B);
		GPIOC->BSRR = (1 << V2_D0_LOW_C) | (1 << V2_D1_LOW_C);

		WAIT_4_CYCLES;

		for(int x = 0;x < 8;x++)
		{
			GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
			GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
			// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
			WAIT_4_CYCLES;
			GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
			GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
			WAIT_4_CYCLES;
		}
	}
	else if(tasrun->console == CONSOLE_SNES)
	{
		// fix bit order

		GPIOB->BSRR = V1_GPIOB_current[8];
		GPIOC->BSRR = V2_GPIOC_current[8];

		// give time to it to register
		WAIT_4_CYCLES;

		GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
		GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
		// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
		WAIT_4_CYCLES;
		GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
		GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
		WAIT_4_CYCLES;

		GPIOB->BSRR = V1_GPIOB_current[0];
		GPIOC->BSRR = V2_GPIOC_current[0];

		// give time to it to register
		WAIT_4_CYCLES;

		GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
		GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
		// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
		WAIT_4_CYCLES;
		GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
		GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
		WAIT_4_CYCLES;

		// at least 10ns in width
		for(int x = 2;x < 8;x++)
		{
			//set vis data
			GPIOB->BSRR = V1_GPIOB_current[x];
			GPIOC->BSRR = V2_GPIOC_current[x];

			// give time to it to register
			WAIT_4_CYCLES;

			GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
			GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
			// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
			WAIT_4_CYCLES;
			GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
			GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
			WAIT_4_CYCLES;
		}

		GPIOB->BSRR = V1_GPIOB_current[9];
		GPIOC->BSRR = V2_GPIOC_current[9];

		// give time to it to register
		WAIT_4_CYCLES;

		GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
		GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
		// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
		WAIT_4_CYCLES;
		GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
		GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
		WAIT_4_CYCLES;

		GPIOB->BSRR = V1_GPIOB_current[1];
		GPIOC->BSRR = V2_GPIOC_current[1];

		// give time to it to register
		WAIT_4_CYCLES;

		GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
		GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
		// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
		WAIT_4_CYCLES;
		GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
		GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
		WAIT_4_CYCLES;

		// at least 10ns in width
		for(int x = 10;x < 16;x++)
		{
			//set vis data
			GPIOB->BSRR = V1_GPIOB_current[x];
			GPIOC->BSRR = V2_GPIOC_current[x];

			// give time to it to register
			WAIT_4_CYCLES;

			GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
			GPIOA->BSRR = (1 << V2_CLOCK_HIGH_A);
			// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
			WAIT_4_CYCLES;
			GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
			GPIOA->BSRR = (1 << V2_CLOCK_LOW_A);
			WAIT_4_CYCLES;
		}

	}
	WAIT_4_CYCLES;

	// create at least a 20ns latch pulse (this should be about 40ns)
	GPIOB->BSRR = (1 << V1_LATCH_HIGH_B);
	GPIOC->BSRR = (1 << V2_LATCH_HIGH_C);
	WAIT_4_CYCLES;
	WAIT_4_CYCLES;
	GPIOB->BSRR = (1 << V1_LATCH_LOW_B);
	GPIOC->BSRR = (1 << V2_LATCH_LOW_C);
}

inline void UpdateN64VisBoards(N64ControllerData n64data)
{
	// use the variable V1_GPIOB_current, which already exists, to store if a button should be pressed or not
	// NOTE: this is not how the variable is used in other functions

	V1_GPIOB_current[0] = 0; // snes vis a
	V1_GPIOB_current[1] = n64data.a; // snes vis b
	V1_GPIOB_current[2] = n64data.l; // snes vis select
	V1_GPIOB_current[3] = n64data.start; // snes vis start
	V1_GPIOB_current[8] = 0; // snes vis x
	V1_GPIOB_current[9] = n64data.b; // snes vis y
	V1_GPIOB_current[10] = n64data.z; // snes vis l
	V1_GPIOB_current[11] = n64data.r; // snes vis r
	V1_GPIOB_current[12] = n64data.c_right; // snes vis 1
	V1_GPIOB_current[13] = n64data.c_down; // snes vis 2
	V1_GPIOB_current[14] = n64data.c_up; // snes vis 3
	V1_GPIOB_current[15] = n64data.c_left; // snes vis 4

	// handle the analog stick to d-pad conversion
	const int8_t ANALOG_THRESHOLD = 40;

	int8_t converted_y = (int8_t)(n64data.y_axis);
	int8_t converted_x = (int8_t)(n64data.x_axis);

	if(converted_x >= 0)
	{
		V1_GPIOB_current[6] = 0; // snes vis left

		if(converted_x >= ANALOG_THRESHOLD)
		{
			V1_GPIOB_current[7] = 1; // snes vis right
		}
		else
		{
			V1_GPIOB_current[7] = 0; // snes vis right
		}
	}
	else
	{
		V1_GPIOB_current[7] = 0; // snes vis right

		if(converted_x <= (-ANALOG_THRESHOLD))
		{
			V1_GPIOB_current[6] = 1; // snes vis left
		}
		else
		{
			V1_GPIOB_current[6] = 0; // snes vis left
		}
	}

	if(converted_y >= 0)
	{
		V1_GPIOB_current[5] = 0; // snes vis down

		if(converted_y >= ANALOG_THRESHOLD)
		{
			V1_GPIOB_current[4] = 1; // snes vis up
		}
		else
		{
			V1_GPIOB_current[4] = 0; // snes vis up
		}
	}
	else
	{
		V1_GPIOB_current[4] = 0; // snes vis up

		if(converted_y <= (-ANALOG_THRESHOLD))
		{
			V1_GPIOB_current[5] = 1; // snes vis down
		}
		else
		{
			V1_GPIOB_current[5] = 0; // snes vis down
		}
	}

	// each bit pulse should be at least 10ns in width
	for(int x = 0;x < 16;x++)
	{
		//set vis d0 line appropriately
		if(V1_GPIOB_current[x] == 1)
		{
			GPIOB->BSRR = 1 << V1_D0_HIGH_B;
		}
		else
		{
			GPIOB->BSRR = 1 << V1_D0_LOW_B;
		}

		// give time to it to register
		WAIT_4_CYCLES;

		// clock it
		GPIOB->BSRR = (1 << V1_CLOCK_HIGH_B);
		// wait 4 cycles which should be well over the minimum required 10ns but still relatively quick
		WAIT_4_CYCLES;
		GPIOB->BSRR = (1 << V1_CLOCK_LOW_B);
		WAIT_4_CYCLES;
	}

	WAIT_4_CYCLES;

	// create at least a 20ns latch pulse (this should be about 40ns)
	GPIOB->BSRR = (1 << V1_LATCH_HIGH_B);
	WAIT_4_CYCLES;
	WAIT_4_CYCLES;
	GPIOB->BSRR = (1 << V1_LATCH_LOW_B);
}

static uint8_t UART2_OutputFunction(uint8_t *buffer, uint16_t n)
{
	return HAL_UART_Transmit_IT(&huart2, buffer, n);
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

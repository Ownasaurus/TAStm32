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
#include "TASrun.h"
#include "usbd_cdc_if.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define P1_D0_HIGH_A  (1 << 8);
#define P1_D0_LOW_A   (1 << 24);
#define P1_D1_HIGH_C  (1 << 3);
#define P1_D1_LOW_C   (1 << 19);
#define P1_D2_HIGH_C  (1 << 14);
#define P1_D2_LOW_C   (1 << 30);
#define P2_D0_HIGH_C  (1 << 6);
#define P2_D0_LOW_C   (1 << 22);
#define P2_D1_HIGH_C  (1 << 9);
#define P2_D1_LOW_C   (1 << 25);
#define P2_D2_HIGH_C  (1 << 8);
#define P2_D2_LOW_C   (1 << 24);
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t p1_d0 = 0;
volatile uint32_t p1_d0_next = 0;
volatile uint32_t p1_d1 = 0;
volatile uint32_t p1_d1_next = 0;
volatile uint32_t p1_d2 = 0;
volatile uint32_t p1_d2_next = 0;

volatile uint32_t p2_d0 = 0;
volatile uint32_t p2_d0_next = 0;
volatile uint32_t p2_d1 = 0;
volatile uint32_t p2_d1_next = 0;
volatile uint32_t p2_d2 = 0;
volatile uint32_t p2_d2_next = 0;

volatile int8_t p1_current_bit = 0;
volatile int8_t p2_current_bit = 0;
volatile uint8_t recentLatch = 0;
Console c = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
void my_wait_us_asm(int n);
void ResetAndEnable8msTimer();
void Disable8msTimer();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern TIM_HandleTypeDef htim3;
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
	// P2_CLOCK

	if(p2_current_bit >= 0) // sanity check... but 32 or more bits should never be read in a single latch!
	{
		uint32_t dataLines = 0;
		if((p2_d0 >> p2_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D0_LOW_C;
		}
		else
		{
			dataLines |= P2_D0_HIGH_C;
		}
		if((p2_d1 >> p2_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D1_LOW_C;
		}
		else
		{
			dataLines |= P2_D1_HIGH_C;
		}
		if((p2_d2 >> p2_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D2_LOW_C;
		}
		else
		{
			dataLines |= P2_D2_HIGH_C;
		}
		GPIOC->BSRR = dataLines;

		p2_current_bit--;
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

	if(recentLatch == 0) // no recent latch
	{
		// COMMENT THIS LINE OUT TO DISABLE DPCM FIX
		//recentLatch = 1;
		ResetAndEnable8msTimer(); // start timer and proceed as normal

		// if first latch, put the data straight in, bypassing the "next" buffer
		if(!GetRunStarted(0))
		{
			RunData (*dataptr)[MAX_CONTROLLERS][MAX_DATA_LANES] = GetNextFrame(0);
			//p1_d0 = dataptr
			memcpy((uint32_t*)&p1_d0, &dataptr[0][0][0], sizeof(RunData));
			memcpy((uint32_t*)&p1_d1, &dataptr[0][0][1], sizeof(RunData));
			memcpy((uint32_t*)&p1_d2, &dataptr[0][0][2], sizeof(RunData));
			memcpy((uint32_t*)&p2_d0, &dataptr[0][1][0], sizeof(RunData));
			memcpy((uint32_t*)&p2_d1, &dataptr[0][1][1], sizeof(RunData));
			memcpy((uint32_t*)&p2_d2, &dataptr[0][1][2], sizeof(RunData));

			// swap bytes due to endainness

			c = TASRunGetConsole(0);

			// prepare overread values based on console
			if(c == CONSOLE_NES) // 8 bit data
			{
				p1_d0 <<= 24;
				p1_d1 <<= 24;
				p1_d2 <<= 24;
				p2_d0 <<= 24;
				p2_d1 <<= 24;
				p2_d2 <<= 24;
				// check 25th bit to determine overread
				if((p1_d0 >> 24) & 1) // if 25th bit is 1
				{
					p1_d0 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p1_d0 &= 0xFF000000; // make the lower bits 0 as well
				}
				if((p1_d1 >> 24) & 1) // if 25th bit is 1
				{
					p1_d1 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p1_d1 &= 0xFF000000; // make the lower bits 0 as well
				}
				if((p1_d2 >> 24) & 1) // if 25th bit is 1
				{
					p1_d2 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p1_d2 &= 0xFF000000; // make the lower bits 0 as well
				}
				if((p2_d0 >> 24) & 1) // if 25th bit is 1
				{
					p2_d0 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p2_d0 &= 0xFF000000; // make the lower bits 0 as well
				}
				if((p2_d1 >> 24) & 1) // if 25th bit is 1
				{
					p2_d1 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p2_d1 &= 0xFF000000; // make the lower bits 0 as well
				}
				if((p2_d2 >> 24) & 1) // if 25th bit is 1
				{
					p2_d2 |= 0x00FFFFFF; // make the lower bits 1 as well
				}
				else // if the 25th bit is 0
				{
					p2_d2 &= 0xFF000000; // make the lower bits 0 as well
				}

				// this can be used to force overread HIGH (0)
				//p1_d0 &= 0xFF000000;

				// this can be used to force overread LOW (1)
				//p1_d0 |= 0x00FFFFFF;
			}
			else if(c == CONSOLE_SNES) // 16 bit data
			{
				p1_d0 = ((p1_d0 >> 8) & 0xFF) | ((p1_d0 << 8) & 0xFF00);
				p1_d1 = ((p1_d1 >> 8) & 0xFF) | ((p1_d1 << 8) & 0xFF00);
				p1_d2 = ((p1_d2 >> 8) & 0xFF) | ((p1_d2 << 8) & 0xFF00);
				p2_d0 = ((p2_d0 >> 8) & 0xFF) | ((p2_d0 << 8) & 0xFF00);
				p2_d1 = ((p2_d1 >> 8) & 0xFF) | ((p2_d1 << 8) & 0xFF00);
				p2_d2 = ((p2_d2 >> 8) & 0xFF) | ((p2_d2 << 8) & 0xFF00);

				p1_d0 <<= 16;
				p1_d1 <<= 16;
				p1_d2 <<= 16;
				p2_d0 <<= 16;
				p2_d1 <<= 16;
				p2_d2 <<= 16;
				// check 17th bit to determine overread
				if((p1_d0 >> 16) & 1) // if 17th bit is 1
				{
					p1_d0 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p1_d0 &= 0xFFFF0000; // make the lower bits 0 as well
				}
				if((p1_d1 >> 16) & 1) // if 17th bit is 1
				{
					p1_d1 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p1_d1 &= 0xFFFF0000; // make the lower bits 0 as well
				}
				if((p1_d2 >> 16) & 1) // if 17th bit is 1
				{
					p1_d2 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p1_d2 &= 0xFFFF0000; // make the lower bits 0 as well
				}
				if((p2_d0 >> 16) & 1) // if 17th bit is 1
				{
					p2_d0 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p2_d0 &= 0xFFFF0000; // make the lower bits 0 as well
				}
				if((p2_d1 >> 16) & 1) // if 17th bit is 1
				{
					p2_d1 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p2_d1 &= 0xFFFF0000; // make the lower bits 0 as well
				}
				if((p2_d2 >> 16) & 1) // if 17th bit is 1
				{
					p2_d2 |= 0x0000FFFF; // make the lower bits 1 as well
				}
				else // if the 17th bit is 0
				{
					p2_d2 &= 0xFFFF0000; // make the lower bits 0 as well
				}

				// this can be used to force overread HIGH (0)
				//p1_d0 &= 0xFFFF0000;

				// this can be used to force overread LOW (1)
				//p1_d0 |= 0x0000FFFF;
			}
			SetRunStarted(0, 1);
		}
		else // otherwise quickly get it from the "next" buffer!
		{
			p1_d0 = p1_d0_next;
			p1_d1 = p1_d1_next;
			p1_d2 = p1_d2_next;
			p2_d0 = p2_d0_next;
			p2_d1 = p2_d1_next;
			p2_d2 = p2_d2_next;
		}

		// write the first bit
		if((p1_d0 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			GPIOA->BSRR = P1_D0_LOW_A;
		}
		else
		{
			GPIOA->BSRR = P1_D0_HIGH_A;
		}
		uint32_t dataLines = 0;
		if((p1_d1 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D1_LOW_C;
		}
		else
		{
			dataLines |= P1_D1_HIGH_C;
		}
		if((p1_d2 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D2_LOW_C;
		}
		else
		{
			dataLines |= P1_D2_HIGH_C;
		}
		if((p2_d0 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D0_LOW_C;
		}
		else
		{
			dataLines |= P2_D0_HIGH_C;
		}
		if((p2_d1 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D1_LOW_C;
		}
		else
		{
			dataLines |= P2_D1_HIGH_C;
		}
		if((p2_d2 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D2_LOW_C;
		}
		else
		{
			dataLines |= P2_D2_HIGH_C;
		}
		GPIOC->BSRR = dataLines;

		p1_current_bit = p2_current_bit = 30; // set the next bit to be read

		// now prepare the next frame!
		RunData (*dataptr)[MAX_CONTROLLERS][MAX_DATA_LANES] = GetNextFrame(0);

		if(dataptr)
		{
			memcpy((uint32_t*)&p1_d0_next, &dataptr[0][0][0], sizeof(RunData));
			memcpy((uint32_t*)&p1_d1_next, &dataptr[0][0][1], sizeof(RunData));
			memcpy((uint32_t*)&p1_d2_next, &dataptr[0][0][2], sizeof(RunData));
			memcpy((uint32_t*)&p2_d0_next, &dataptr[0][1][0], sizeof(RunData));
			memcpy((uint32_t*)&p2_d1_next, &dataptr[0][1][1], sizeof(RunData));
			memcpy((uint32_t*)&p2_d2_next, &dataptr[0][1][2], sizeof(RunData));
		}
		else
		{
			memset((uint32_t*)&p1_d0_next, 0, sizeof(RunData));
			memset((uint32_t*)&p1_d1_next, 0, sizeof(RunData));
			memset((uint32_t*)&p1_d2_next, 0, sizeof(RunData));
			memset((uint32_t*)&p2_d0_next, 0, sizeof(RunData));
			memset((uint32_t*)&p2_d1_next, 0, sizeof(RunData));
			memset((uint32_t*)&p2_d2_next, 0, sizeof(RunData));
		}

		c = TASRunGetConsole(0);

		// prepare overread values based on console
		if(c == CONSOLE_NES) // 8 bit data
		{
			p1_d0_next <<= 24;
			p1_d1_next <<= 24;
			p1_d2_next <<= 24;
			p2_d0_next <<= 24;
			p2_d1_next <<= 24;
			p2_d2_next <<= 24;
			// check 25th bit to determine overread
			if((p1_d0_next >> 24) & 1) // if 25th bit is 1
			{
				p1_d0_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p1_d0_next &= 0xFF000000; // make the lower bits 0 as well
			}
			if((p1_d1_next >> 24) & 1) // if 25th bit is 1
			{
				p1_d1_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p1_d1_next &= 0xFF000000; // make the lower bits 0 as well
			}
			if((p1_d2_next >> 24) & 1) // if 25th bit is 1
			{
				p1_d2_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p1_d2_next &= 0xFF000000; // make the lower bits 0 as well
			}
			if((p2_d0_next >> 24) & 1) // if 25th bit is 1
			{
				p2_d0_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p2_d0_next &= 0xFF000000; // make the lower bits 0 as well
			}
			if((p2_d1_next >> 24) & 1) // if 25th bit is 1
			{
				p2_d1_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p2_d1_next &= 0xFF000000; // make the lower bits 0 as well
			}
			if((p2_d2_next >> 24) & 1) // if 25th bit is 1
			{
				p2_d2_next |= 0x00FFFFFF; // make the lower bits 1 as well
			}
			else // if the 25th bit is 0
			{
				p2_d2_next &= 0xFF000000; // make the lower bits 0 as well
			}

			// this can be used to force overread HIGH (0)
			//p1_d0_next &= 0xFF000000;

			// this can be used to force overread LOW (1)
			//p1_d0_next |= 0x00FFFFFF;
		}
		else if(c == CONSOLE_SNES) // 16 bit data
		{
			p1_d0_next = ((p1_d0_next >> 8) & 0xFF) | ((p1_d0_next << 8) & 0xFF00);
			p1_d1_next = ((p1_d1_next >> 8) & 0xFF) | ((p1_d1_next << 8) & 0xFF00);
			p1_d2_next = ((p1_d2_next >> 8) & 0xFF) | ((p1_d2_next << 8) & 0xFF00);
			p2_d0_next = ((p2_d0_next >> 8) & 0xFF) | ((p2_d0_next << 8) & 0xFF00);
			p2_d1_next = ((p2_d1_next >> 8) & 0xFF) | ((p2_d1_next << 8) & 0xFF00);
			p2_d2_next = ((p2_d2_next >> 8) & 0xFF) | ((p2_d2_next << 8) & 0xFF00);

			p1_d0_next <<= 16;
			p1_d1_next <<= 16;
			p1_d2_next <<= 16;
			p2_d0_next <<= 16;
			p2_d1_next <<= 16;
			p2_d2_next <<= 16;
			// check 17th bit to determine overread
			if((p1_d0_next >> 16) & 1) // if 17th bit is 1
			{
				p1_d0_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p1_d0_next &= 0xFFFF0000; // make the lower bits 0 as well
			}
			if((p1_d1_next >> 16) & 1) // if 17th bit is 1
			{
				p1_d1_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p1_d1_next &= 0xFFFF0000; // make the lower bits 0 as well
			}
			if((p1_d2_next >> 16) & 1) // if 17th bit is 1
			{
				p1_d2_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p1_d2_next &= 0xFFFF0000; // make the lower bits 0 as well
			}
			if((p2_d0_next >> 16) & 1) // if 17th bit is 1
			{
				p2_d0_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p2_d0_next &= 0xFFFF0000; // make the lower bits 0 as well
			}
			if((p2_d1_next >> 16) & 1) // if 17th bit is 1
			{
				p2_d1_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p2_d1_next &= 0xFFFF0000; // make the lower bits 0 as well
			}
			if((p2_d2_next >> 16) & 1) // if 17th bit is 1
			{
				p2_d2_next |= 0x0000FFFF; // make the lower bits 1 as well
			}
			else // if the 17th bit is 0
			{
				p2_d2_next &= 0xFFFF0000; // make the lower bits 0 as well
			}

			// this can be used to force overread HIGH (0)
			//p1_d0_next &= 0xFFFF0000;

			// this can be used to force overread LOW (1)
			//p1_d0_next |= 0x0000FFFF;
		}

		if(!dataptr) // notify buffer underflow
		{
			CDC_Transmit_FS((uint8_t*)"\xB2", 1); // notify buffer underflow
		}

		CDC_Transmit_FS((uint8_t*)"A", 1); // notify that we latched
	}
	else // we latched very recently
	{
		//TODO: write for all registers
		// write the first bit
		if((p1_d0 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			GPIOA->BSRR = P1_D0_LOW_A;
		}
		else
		{
			GPIOA->BSRR = P1_D0_HIGH_A;
		}
		uint32_t dataLines = 0;
		if((p1_d1 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D1_LOW_C;
		}
		else
		{
			dataLines |= P1_D1_HIGH_C;
		}
		if((p1_d2 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D2_LOW_C;
		}
		else
		{
			dataLines |= P1_D2_HIGH_C;
		}
		if((p2_d0 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D0_LOW_C;
		}
		else
		{
			dataLines |= P2_D0_HIGH_C;
		}
		if((p2_d1 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D1_LOW_C;
		}
		else
		{
			dataLines |= P2_D1_HIGH_C;
		}
		if((p2_d2 >> 31) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P2_D2_LOW_C;
		}
		else
		{
			dataLines |= P2_D2_HIGH_C;
		}
		GPIOC->BSRR = dataLines;

		p1_current_bit = p2_current_bit = 30; // reset back to beginning of "frame" and do not advance
	}

  /* USER CODE END EXTI1_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1);
  /* USER CODE BEGIN EXTI1_IRQn 1 */

  /* USER CODE END EXTI1_IRQn 1 */
}

/**
  * @brief This function handles EXTI line 2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI2_IRQn 0 */
	// P1_CLOCK

	if(p1_current_bit >= 0) // sanity check... but 32 or more bits should never be read in a single latch!
	{
		if((p1_d0 >> p1_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			GPIOA->BSRR = P1_D0_LOW_A;
		}
		else
		{
			GPIOA->BSRR = P1_D0_HIGH_A;
		}
		uint32_t dataLines = 0;
		if((p1_d1 >> p1_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D1_LOW_C;
		}
		else
		{
			dataLines |= P1_D1_HIGH_C;
		}
		if((p1_d2 >> p1_current_bit) & 1) // if the button is PRESSED, set the line LOW
		{
			dataLines |= P1_D2_LOW_C;
		}
		else
		{
			dataLines |= P1_D2_HIGH_C;
		}
		GPIOC->BSRR = dataLines;

		p1_current_bit--;
	}

  /* USER CODE END EXTI2_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
  /* USER CODE BEGIN EXTI2_IRQn 1 */

  /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
	// Read 64 command
	__disable_irq();
	uint32_t cmd;
	RunData (*frame)[MAX_CONTROLLERS][MAX_DATA_LANES];

	cmd = readCommand();

	my_wait_us_asm(2); // wait a small amount of time before replying

	//-------- SEND RESPONSE
	SetP1Data0OutputMode();

	switch(cmd)
	{
	  case 0x00: // identity
	  case 0xFF: // N64 reset
		  SendIdentityN64();
		  break;
	  case 0x01: // poll for N64 state
		  frame = GetNextFrame(0);
		  if(frame == NULL) // buffer underflow
		  {
			  SendControllerDataN64(0); // send blank controller data
		  }
		  else
		  {
			  SendControllerDataN64((*frame)[0][0]);
		  }
		  break;
	  case 0x02:
	  case 0x03:
	  default:
		  // we do not process the read and write commands (memory pack)
		  break;
	}
	//-------- DONE SENDING RESPOSE

	SetP1Data0InputMode();

	__enable_irq();

	if(cmd == 0x01)
	{
		CDC_Transmit_FS((uint8_t*)"A", 1);

		if(frame == NULL) // there was a buffer underflow
			CDC_Transmit_FS((uint8_t*)"\xB2", 1);
	}

  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

	recentLatch = 0;
	Disable8msTimer(); // to ensure it was a 1-shot

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
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

/* USER CODE BEGIN 1 */
void Disable8msTimer()
{
	TIM3->CNT = 0; // reset count
	TIM3->SR = 0; // reset flags

	HAL_TIM_Base_Stop_IT(&htim3);
}

void ResetAndEnable8msTimer()
{
	TIM3->CNT = 0; // reset count
	TIM3->SR = 0; // reset flags

	HAL_TIM_Base_Start_IT(&htim3);
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

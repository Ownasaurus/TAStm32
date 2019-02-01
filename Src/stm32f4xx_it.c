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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const uint8_t P1_D0_HIGH_A = 8;
const uint8_t P1_D0_LOW_A = 24;
const uint8_t P1_D1_HIGH_C = 3;
const uint8_t P1_D1_LOW_C = 19;
const uint8_t P1_D2_HIGH_C = 4;
const uint8_t P1_D2_LOW_C = 20;
const uint8_t P2_D0_HIGH_C = 6;
const uint8_t P2_D0_LOW_C = 22;
const uint8_t P2_D1_HIGH_C = 9;
const uint8_t P2_D1_LOW_C = 25;
const uint8_t P2_D2_HIGH_C = 8;
const uint8_t P2_D2_LOW_C = 24;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
volatile uint32_t p1_d0_next = 0;
volatile uint32_t p1_d1_next = 0;
volatile uint32_t p1_d2_next = 0;
volatile uint32_t p2_d0_next = 0;
volatile uint32_t p2_d1_next = 0;
volatile uint32_t p2_d2_next = 0;

// leave enough room for SNES + overread
volatile uint32_t P1_GPIOA_current[32];
volatile uint32_t P1_GPIOA_next[32];
volatile uint32_t P1_GPIOC_current[32];
volatile uint32_t P1_GPIOC_next[32];

volatile uint32_t P2_GPIOC_current[32];
volatile uint32_t P2_GPIOC_next[32];

volatile uint8_t p1_current_bit = 0;
volatile uint8_t p2_current_bit = 0;

volatile uint8_t recentLatch = 0;
volatile uint8_t toggleNext = 0;
volatile uint8_t dpcmFix = 0;
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

	if(p2_current_bit < 32) // sanity check... but 32 or more bits should never be read in a single latch!
	{
		GPIOC->BSRR = P2_GPIOC_current[p2_current_bit];

		p2_current_bit++;
	}
  /* USER CODE END EXTI0_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
  /* USER CODE BEGIN EXTI0_IRQn 1 */

  /* USER CODE END EXTI0_IRQn 1 */
}

/**
  * @brief This function handles EXTI line 1 interrupt.
  */
__attribute__((optimize("unroll-loops")))
void EXTI1_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI1_IRQn 0 */
	// P1_LATCH

	if(recentLatch == 0) // no recent latch
	{
		GPIOC->BSRR = P1_GPIOC_next[0] | P2_GPIOC_next[0];
		GPIOA->BSRR = P1_GPIOA_next[0];

		memcpy((uint32_t*)&P1_GPIOA_current, (uint32_t*)&P1_GPIOA_next, 128);
		memcpy((uint32_t*)&P1_GPIOC_current, (uint32_t*)&P1_GPIOC_next, 128);

		memcpy((uint32_t*)&P2_GPIOC_current, (uint32_t*)&P2_GPIOC_next, 128);

		p1_current_bit = p2_current_bit = 1; // set the next bit to be read

		// now prepare for the next frame!

		if(toggleNext)
		{
			dpcmFix = 1 - dpcmFix;
		}

		if(dpcmFix)
		{
			recentLatch = 1;
			ResetAndEnable8msTimer(); // start timer and proceed as normal
		}

		toggleNext = TASRunIncrementFrameCount(0);

		RunData (*dataptr)[MAX_CONTROLLERS][MAX_DATA_LANES] = GetNextFrame(0);

		if(dataptr)
		{
			memcpy((uint32_t*)&p1_d0_next, &dataptr[0][0][0], sizeof(RunData));
			memcpy((uint32_t*)&p1_d1_next, &dataptr[0][0][1], sizeof(RunData));
			memcpy((uint32_t*)&p1_d2_next, &dataptr[0][0][2], sizeof(RunData));
			memcpy((uint32_t*)&p2_d0_next, &dataptr[0][1][0], sizeof(RunData));
			memcpy((uint32_t*)&p2_d1_next, &dataptr[0][1][1], sizeof(RunData));
			memcpy((uint32_t*)&p2_d2_next, &dataptr[0][1][2], sizeof(RunData));

			c = TASRunGetConsole(0);

			int8_t databit = 0;
			if(c == CONSOLE_NES)
			{
				databit = 7; // number of bits of NES - 1
			}
			else
			{
				databit = 15; // number of bits of SNES - 1

				// fix endianness
				p1_d0_next = ((p1_d0_next >> 8) & 0xFF) | ((p1_d0_next << 8) & 0xFF00);
				p1_d1_next = ((p1_d1_next >> 8) & 0xFF) | ((p1_d1_next << 8) & 0xFF00);
				p1_d2_next = ((p1_d2_next >> 8) & 0xFF) | ((p1_d2_next << 8) & 0xFF00);
				p2_d0_next = ((p2_d0_next >> 8) & 0xFF) | ((p2_d0_next << 8) & 0xFF00);
				p2_d1_next = ((p2_d1_next >> 8) & 0xFF) | ((p2_d1_next << 8) & 0xFF00);
				p2_d2_next = ((p2_d2_next >> 8) & 0xFF) | ((p2_d2_next << 8) & 0xFF00);
			}


			int8_t regbit = 0;

			// fill the regular data
			while(databit >= 0)
			{
				P1_GPIOA_next[regbit] = (((p1_d0_next >> databit) & 1) << P1_D0_HIGH_A);
				P1_GPIOA_next[regbit] += (((~P1_GPIOA_next[regbit]) & 0x0100) << 16);
				P1_GPIOC_next[regbit] = (((p1_d1_next >> databit) & 1) << P1_D1_HIGH_C) |
										(((p1_d2_next >> databit) & 1) << P1_D2_HIGH_C);
				P1_GPIOC_next[regbit] += (((~P1_GPIOC_next[regbit]) & 0x0018) << 16);
				P2_GPIOC_next[regbit] = (((p2_d0_next >> databit) & 1) << P2_D0_HIGH_C) |
										(((p2_d1_next >> databit) & 1) << P2_D1_HIGH_C) |
										(((p2_d2_next >> databit) & 1) << P2_D2_HIGH_C);
				P2_GPIOC_next[regbit] += (((~P2_GPIOC_next[regbit]) & 0x0340) << 16);

				regbit++;
				databit--;
			}

			// fill the overread
			if(TASRunGetOverread(0)) // overread is 1/HIGH
			{
				// so set logical LOW (button pressed)
				for(uint8_t index = regbit;index < 32;index++)
				{
					P1_GPIOA_next[index] = (1 << P1_D0_LOW_A);
					P1_GPIOC_next[index] = (1 << P1_D1_LOW_C) | (1 << P1_D2_LOW_C);
					P2_GPIOC_next[index] = (1 << P2_D0_LOW_C) | (1 << P2_D1_LOW_C) | (1 << P2_D2_LOW_C);
				}
			}
			else
			{
				for(uint8_t index = regbit;index < 32;index++)
				{
					P1_GPIOA_next[index] = (1 << P1_D0_HIGH_A);
					P1_GPIOC_next[index] = (1 << P1_D1_HIGH_C) | (1 << P1_D2_HIGH_C);
					P2_GPIOC_next[index] = (1 << P2_D0_HIGH_C) | (1 << P2_D1_HIGH_C) | (1 << P2_D2_HIGH_C);
				}
			}
		}
		else // no data left in the buffer
		{
			// no controller data means all pins get set high for this protocol
			for(uint8_t index = 0;index < 32;index++)
			{
				P1_GPIOA_next[index] = (1 << P1_D0_HIGH_A);
				P1_GPIOC_next[index] = (1 << P1_D1_HIGH_C) | (1 << P1_D2_HIGH_C);
				P2_GPIOC_next[index] = (1 << P2_D0_HIGH_C) | (1 << P2_D1_HIGH_C) | (1 << P2_D2_HIGH_C);
			}
		}

		if(!dataptr) // notify buffer underflow
		{
			CDC_Transmit_FS((uint8_t*)"\xB2", 1); // notify buffer underflow
		}

		if(!TASRunReadyToPreBuffer(0))
		{
			CDC_Transmit_FS((uint8_t*)"A", 1); // notify that we latched
		}
	}
	else // multiple close latches and DPCM fix is enabled
	{
		// repeat the same frame of input
		GPIOC->BSRR = P1_GPIOC_current[0] | P2_GPIOC_current[0];
		GPIOA->BSRR = P1_GPIOA_current[0];

		p1_current_bit = p2_current_bit = 1;
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

	if(p1_current_bit < 32) // sanity check... but 32 or more bits should never be read in a single latch!
	{
		GPIOC->BSRR = P1_GPIOC_current[p1_current_bit];
		GPIOA->BSRR = P1_GPIOA_current[p1_current_bit];

		p1_current_bit++;
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
			  SendControllerDataN64(frame[0][0][0]);
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

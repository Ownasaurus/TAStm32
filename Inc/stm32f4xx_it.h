/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
// The above description is inappropriate for how we're using it.
// TODO: replace with atomics?
extern volatile uint8_t p1_current_bit;
extern volatile uint8_t p2_current_bit;
extern volatile uint8_t recentLatch;
extern volatile uint8_t toggleNext;
extern volatile uint8_t dpcmFix;
extern volatile uint8_t clockFix;
extern volatile uint32_t P1_GPIOC_current[17];
extern volatile uint32_t P1_GPIOC_next[17];
extern volatile uint32_t P2_GPIOC_current[17];
extern volatile uint32_t P2_GPIOC_next[17];
extern volatile uint32_t V1_GPIOB_current[16];
extern volatile uint32_t V1_GPIOB_next[16];
extern volatile uint32_t V2_GPIOC_current[16];
extern volatile uint32_t V2_GPIOC_next[16];
extern const uint8_t SNES_RESET_HIGH_A;
extern const uint8_t SNES_RESET_LOW_A;

extern uint8_t request_pending;
extern uint8_t bulk_mode;

extern uint16_t current_train_index;
extern uint16_t current_train_latch_count;
extern uint8_t between_trains;
extern uint16_t* latch_trains;
extern uint8_t trains_enabled;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void SysTick_Handler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_UP_TIM10_IRQHandler(void);
void TIM3_IRQHandler(void);
void USART2_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void TIM7_IRQHandler(void);
void OTG_FS_IRQHandler(void);
/* USER CODE BEGIN EFP */
void DisableTrainTimer();
void Disable8msTimer();
void DisableP1ClockTimer();
void DisableP2ClockTimer();
void ResetAndEnableTrainTimer();
void ResetAndEnable8msTimer();
void ResetAndEnableP1ClockTimer();
void ResetAndEnableP2ClockTimer();
void UpdateVisBoards();
/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

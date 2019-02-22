/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void ReInitClockTimers(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define P2_CLOCK_Pin GPIO_PIN_0
#define P2_CLOCK_GPIO_Port GPIOC
#define P2_CLOCK_EXTI_IRQn EXTI0_IRQn
#define P1_LATCH_Pin GPIO_PIN_1
#define P1_LATCH_GPIO_Port GPIOC
#define P1_LATCH_EXTI_IRQn EXTI1_IRQn
#define P1_CLOCK_Pin GPIO_PIN_2
#define P1_CLOCK_GPIO_Port GPIOC
#define P1_CLOCK_EXTI_IRQn EXTI2_IRQn
#define P1_DATA_1_Pin GPIO_PIN_3
#define P1_DATA_1_GPIO_Port GPIOC
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define P1_DATA_2_Pin GPIO_PIN_4
#define P1_DATA_2_GPIO_Port GPIOC
#define P2_DATA_2_Pin GPIO_PIN_6
#define P2_DATA_2_GPIO_Port GPIOC
#define P2_DATA_0_Pin GPIO_PIN_7
#define P2_DATA_0_GPIO_Port GPIOC
#define P2_DATA_1_Pin GPIO_PIN_8
#define P2_DATA_1_GPIO_Port GPIOC
#define P2_LATCH_Pin GPIO_PIN_9
#define P2_LATCH_GPIO_Port GPIOC
#define P2_LATCH_EXTI_IRQn EXTI9_5_IRQn
#define P1_DATA_0_Pin GPIO_PIN_8
#define P1_DATA_0_GPIO_Port GPIOA
#define P1_DATA_0_EXTI_IRQn EXTI9_5_IRQn
#define SNES_RESET_Pin GPIO_PIN_9
#define SNES_RESET_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define V2_CLOCK_Pin GPIO_PIN_15
#define V2_CLOCK_GPIO_Port GPIOA
#define V2_LATCH_Pin GPIO_PIN_10
#define V2_LATCH_GPIO_Port GPIOC
#define V2_DATA_1_Pin GPIO_PIN_11
#define V2_DATA_1_GPIO_Port GPIOC
#define V2_DATA_0_Pin GPIO_PIN_12
#define V2_DATA_0_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define V1_CLOCK_Pin GPIO_PIN_4
#define V1_CLOCK_GPIO_Port GPIOB
#define V1_LATCH_Pin GPIO_PIN_5
#define V1_LATCH_GPIO_Port GPIOB
#define V1_DATA_1_Pin GPIO_PIN_6
#define V1_DATA_1_GPIO_Port GPIOB
#define V1_DATA_0_Pin GPIO_PIN_7
#define V1_DATA_0_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

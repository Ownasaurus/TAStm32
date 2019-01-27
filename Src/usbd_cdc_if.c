/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "TASRun.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern volatile uint8_t recentLatch;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:
        pbuf[0] = (uint8_t)BAUD_RATE;
        pbuf[1] = (uint8_t)(BAUD_RATE >> 8);
        pbuf[2] = (uint8_t)(BAUD_RATE >> 16);
        pbuf[3] = (uint8_t)(BAUD_RATE >> 24);
        pbuf[4] = CHAR_FORMAT;
        pbuf[5] = PARITY_TYPE;
        pbuf[6] = NUMBER_DATA_BITS;

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
	static SerialState ss = SERIAL_PREFIX;
	static SerialRun sr = RUN_NONE;
	RunData frame[MAX_CONTROLLERS][MAX_DATA_LANES];
	uint8_t val;

	for(int byteNum = 0;byteNum < *Len;byteNum++)
	{
		switch(ss)
		{
			case SERIAL_COMPLETE: // in case more than 1 command is sent at a time
			case SERIAL_PREFIX:
				switch(Buf[byteNum])
				{
					case 'R': // Reset/clear all configuration

						// disable interrupts on latch/clock/data for now
						HAL_NVIC_DisableIRQ(EXTI0_IRQn);
						HAL_NVIC_DisableIRQ(EXTI1_IRQn);
						HAL_NVIC_DisableIRQ(EXTI2_IRQn);
						HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);

						// clear all interrupts
						while (HAL_NVIC_GetPendingIRQ(EXTI0_IRQn))
						{
							__HAL_GPIO_EXTI_CLEAR_IT(P2_CLOCK_Pin);
							HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
						}
						while (HAL_NVIC_GetPendingIRQ(EXTI1_IRQn))
						{
							__HAL_GPIO_EXTI_CLEAR_IT(P1_LATCH_Pin);
							HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
						}
						while (HAL_NVIC_GetPendingIRQ(EXTI2_IRQn))
						{
							__HAL_GPIO_EXTI_CLEAR_IT(P1_CLOCK_Pin);
							HAL_NVIC_ClearPendingIRQ(EXTI2_IRQn);
						}
						while (HAL_NVIC_GetPendingIRQ(EXTI9_5_IRQn))
						{
							__HAL_GPIO_EXTI_CLEAR_IT(P1_DATA_0_Pin);
							__HAL_GPIO_EXTI_CLEAR_IT(P2_LATCH_Pin);
							HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
						}
						while (HAL_NVIC_GetPendingIRQ(TIM3_IRQn))
						{
							HAL_NVIC_ClearPendingIRQ(TIM3_IRQn);
						}

						// important to reset our state
						recentLatch = 0;

						SetRunStarted(0, 0); // mark run as not yet started

						ResetTASRuns();
						CDC_Transmit_FS((uint8_t*)"\x01R", 2); // good response for reset
						ss = SERIAL_COMPLETE;
						break;
					case 'A': // Run #1 controller data
						sr = RUN_A;
						ss = SERIAL_CONTROLLER_DATA;
						break;
					case 'B': // Run #2 controller data
						sr = RUN_B;
						ss = SERIAL_CONTROLLER_DATA;
						break;
					case 'C': // Run #3 controller data
						sr = RUN_C;
						ss = SERIAL_CONTROLLER_DATA;
						break;
					case 'D': // Run #4 controller data
						sr = RUN_D;
						ss = SERIAL_CONTROLLER_DATA;
						break;
					case 'S': // Setup a run
						ss = SERIAL_SETUP;
						break;
					default: // Error: prefix not understood
						CDC_Transmit_FS((uint8_t*)"\xFF", 1);
						break;
				}
				break;
			case SERIAL_CONTROLLER_DATA:
				ExtractDataAndAdvance(&frame, sr, Buf, &byteNum);
				if(AddFrame(sr, &frame) == 0) // buffer must have been full
				{
					CDC_Transmit_FS((uint8_t*)"\xB0", 1);
				}
				ss = SERIAL_COMPLETE;
				sr = RUN_NONE;
				break;
			case SERIAL_CONSOLE:
				switch(Buf[byteNum])
				{
					case 'M': // setup N64
						TASRunSetConsole(sr, CONSOLE_N64);
						SetP1Data0InputMode();
						ss = SERIAL_NUM_CONTROLLERS;
						break;
					case 'S': // setup SNES
						TASRunSetConsole(sr, CONSOLE_SNES);
						SetP1Data0OutputMode();
						ss = SERIAL_NUM_CONTROLLERS;
						break;
					case 'N': // setup NES
						TASRunSetConsole(sr, CONSOLE_NES);
						SetP1Data0OutputMode();
						ss = SERIAL_NUM_CONTROLLERS;
						break;
					default: // Error: console type not understood
						CDC_Transmit_FS((uint8_t*)"\xFC", 1);
						break;
				}
				break;
			case SERIAL_SETUP:
				switch(Buf[byteNum])
				{
					case 'A': // setup Run #1
						sr = RUN_A;
						ss = SERIAL_CONSOLE;
						break;
					case 'B': // setup Run #2
						sr = RUN_B;
						ss = SERIAL_CONSOLE;
						break;
					case 'C': // setup Run #3
						sr = RUN_C;
						ss = SERIAL_CONSOLE;
						break;
					case 'D': // setup Run #4
						sr = RUN_D;
						ss = SERIAL_CONSOLE;
						break;
					default: // Error: run number not understood
						CDC_Transmit_FS((uint8_t*)"\xFE", 1);
						break;
				}
				break;
			case SERIAL_NUM_CONTROLLERS:
				//TODO: actually get correct players/controllers instead of counting bits

				val = Buf[byteNum];
				uint8_t p1 = (val >> 4);
				uint8_t p2 = (val & 0xF);
				uint8_t p1_lanes = 0, p2_lanes = 0;

				if(p1 == 0x8)
				{
					p1_lanes = 1;
				}
				else if(p1 == 0xC)
				{
					p1_lanes = 2;
				}
				else if(p1 == 0xE)
				{
					p1_lanes = 3;
				}

				if(p2 == 0x8)
				{
					p2_lanes = 1;
				}
				else if(p2 == 0xC)
				{
					p2_lanes = 2;
				}
				else if(p2 == 0xE)
				{
					p2_lanes = 3;
				}

				if(p1 != 0) // player 1 better have some kind of data!
				{
					if(p2 != 0) // 2 controllers
					{
						TASRunSetNumControllers(sr, 2);

						if(p1_lanes == p2_lanes)
						{
							TASRunSetNumDataLanes(sr, p1_lanes);
						}
						else // error
						{
							CDC_Transmit_FS((uint8_t*)"\xFD", 1);
						}
					}
					else // 1 controller
					{
						TASRunSetNumControllers(sr, 1);
						TASRunSetNumDataLanes(sr, p1_lanes);
					}
				}
				else
				{
					CDC_Transmit_FS((uint8_t*)"\xFD", 1);
				}

				ss = SERIAL_SETTINGS;
				break;
			case SERIAL_SETTINGS:
				//TODO: read settings byte
				CDC_Transmit_FS((uint8_t*)"\x01S", 2);

				// enable interrupts as needed
				Console c = TASRunGetConsole(0);

				if(c == CONSOLE_NES || c == CONSOLE_SNES)
				{
					HAL_NVIC_EnableIRQ(EXTI0_IRQn);
					HAL_NVIC_EnableIRQ(EXTI1_IRQn);
					HAL_NVIC_EnableIRQ(EXTI2_IRQn);
				}
				else if(c == CONSOLE_N64)
				{
					HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
				}

				ss = SERIAL_COMPLETE;
				sr = RUN_NONE;
				break;
			default:
				break;
		}
	}

  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);

  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
#include "TASRun.h"
#include "stm32f4xx_it.h"
#include "serial_interface.h"
#include "main.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern uint8_t p1_current_bit;
extern uint8_t p2_current_bit;
extern volatile uint8_t recentLatch;
extern volatile uint8_t toggleNext;
extern volatile uint8_t dpcmFix;
extern volatile uint8_t clockFix;
extern uint32_t P1_GPIOC_current[17];
extern volatile uint32_t P1_GPIOC_next[17];
extern uint32_t P2_GPIOC_current[17];
extern volatile uint32_t P2_GPIOC_next[17];
extern volatile uint32_t V1_GPIOB_current[16];
extern volatile uint32_t V1_GPIOB_next[16];
extern volatile uint32_t V2_GPIOC_current[16];
extern volatile uint32_t V2_GPIOC_next[16];
extern uint8_t jumpToDFU;
extern const uint8_t SNES_RESET_HIGH_A;
extern const uint8_t SNES_RESET_LOW_A;

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

uint8_t request_pending = 0;
uint8_t bulk_mode = 0;

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
static uint8_t controller_data_buffer[sizeof(RunData) * MAX_CONTROLLERS * MAX_DATA_LANES];
static uint8_t controller_data_bytes_read = 0;

static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
	serial_interface_set_output_function(CDC_Transmit_FS);

	static SerialInterfaceState ss = SERIAL_PREFIX;
	static SerialRun sr = RUN_NONE;
	uint8_t val;
	static uint8_t console_size = 0;
	static uint8_t input_size = 0;

	for(int byteNum = 0;byteNum < *Len;byteNum++)
	{
		switch(ss)
		{
			case SERIAL_COMPLETE: // in case more than 1 command is sent at a time
			case SERIAL_PREFIX:
				switch(Buf[byteNum])
				{
					case 'L':
						ss = SERIAL_LANE;
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
						CDC_Transmit_FS((uint8_t*)"\x01R", 2); // good response for reset
						ss = SERIAL_COMPLETE;
						break;
					case 'A': // Run #1 controller data
						sr = RUN_A;
						ss = SERIAL_CONTROLLER_DATA_START;
						break;
					case 'a': // 28 frame data burst is complete
						request_pending = 0;
						break;
					case 'Q':
						ss = SERIAL_CMD_Q_1;
						break;
					case 'S': // Setup a run
						ss = SERIAL_SETUP;
						break;
					case 'T': // Transition
						ss = SERIAL_TRANSITION;
						break;
					case 'P': // Power controls
						ss = SERIAL_POWER;
						break;
					case '\xDF':
						jumpToDFU = 1;
						break;
					default: // Error: prefix not understood
						CDC_Transmit_FS((uint8_t*)"\xFF", 1);
						break;
				}
				break;
			case SERIAL_CMD_Q_1:
				if(Buf[byteNum] != 'A')
				{
					CDC_Transmit_FS((uint8_t*)"\xFE", 1); // run not supported
					sr = RUN_NONE;
					ss = SERIAL_COMPLETE;
				} else {
					ss = SERIAL_CMD_Q_2;
				}
				break;
			case SERIAL_CMD_Q_2:
				if (Buf[byteNum] == '1') // enter bulk transfer mode
				{
					bulk_mode = 1;
				} else if (Buf[byteNum] == '0') // exit bulk transfer mode
				{
					bulk_mode = 0;
				} else // should not reach this
				{
					CDC_Transmit_FS((uint8_t*) "\xFA", 1); // Error during bulk transfer toggle
					sr = RUN_NONE;
				}
				ss = SERIAL_COMPLETE;
				break;
			case SERIAL_LANE:
				if(Buf[byteNum] == 'A')
				{
					EXTI1_IRQHandler(); // simulate that a latch has occurred
				}

				ss = SERIAL_COMPLETE;
				break;
			case SERIAL_POWER:
				switch(Buf[byteNum])
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
						ss = SERIAL_COMPLETE;
						break;
				}
				ss = SERIAL_COMPLETE;
				break;
			case SERIAL_CONTROLLER_DATA_START:
				controller_data_bytes_read = 0;
				ss = SERIAL_CONTROLLER_DATA_CONTINUE;
				// fall through
			case SERIAL_CONTROLLER_DATA_CONTINUE:
				controller_data_buffer[controller_data_bytes_read++] = Buf[byteNum];
				if (controller_data_bytes_read < 8)
				{
					// wait for next byte...
					break;
				}

				if (ExtractDataAndAddFrame(sr, controller_data_buffer, controller_data_bytes_read) == 0)
				{
					// buffer must have been full
					CDC_Transmit_FS((uint8_t*)"\xB0", 1);
				}

				if(!TASRunIsInitialized(0) && TASRunGetSize(0) > 0) // this should only run once per run to set up the 1st frame of data
				{
					if(TASRunGetDPCMFix(0))
					{
						toggleNext = 1;
					}
					if(TASRunGetClockFix(0))
					{
						clockFix = 1;
					}

					Console c = TASRunGetConsole(0);

					// thanks to booto for this logic fix
					if(c == CONSOLE_NES || c == CONSOLE_SNES) // needed to prime the buffer for NES/SNES
					{
						EXTI1_IRQHandler();
					}

					TASRunSetInitialized(0, 1);

					// enable interrupts as needed
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

				ss = SERIAL_COMPLETE;
				sr = RUN_NONE;
				break;
			case SERIAL_CONSOLE:
				switch(Buf[byteNum])
				{
					case 'M': // setup N64
						TASRunSetConsole(sr, CONSOLE_N64);
						SetN64Mode();
						ss = SERIAL_NUM_CONTROLLERS;
						console_size = 4;
						break;
					case 'G': // setup Gamecube
						TASRunSetConsole(sr, CONSOLE_GC);
						SetN64Mode();
						ss = SERIAL_NUM_CONTROLLERS;
						console_size = 8;
						break;
					case 'S': // setup SNES
						TASRunSetConsole(sr, CONSOLE_SNES);
						SetSNESMode();
						ss = SERIAL_NUM_CONTROLLERS;
						console_size = 2;
						break;
					case 'N': // setup NES
						TASRunSetConsole(sr, CONSOLE_NES);
						SetSNESMode();
						ss = SERIAL_NUM_CONTROLLERS;
						console_size = 1;
						break;
					default: // Error: console type not understood
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
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
					default: // Error: run number not understood
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
						CDC_Transmit_FS((uint8_t*)"\xFE", 1);
						break;
				}
				break;
			case SERIAL_NUM_CONTROLLERS:
				val = Buf[byteNum];
				uint8_t p1 = (val >> 4);
				uint8_t p2 = (val & 0xF);
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
					ss = SERIAL_COMPLETE;
					sr = RUN_NONE;
					CDC_Transmit_FS((uint8_t*)"\xFD", 1);
					break;
				}

				ss = SERIAL_SETTINGS;
				input_size = GetSizeOfInputForRun(sr);
				break;
			case SERIAL_SETTINGS:
				val = Buf[byteNum];

				TASRunSetDPCMFix(0, ((val >> 7) & 1));
				TASRunSetOverread(0, ((val >> 6) & 1));
				// acceptable values for clock fix: 0 --> 63
				// effective range of clock fix timer: 0us --> 15.75 us
				TASRunSetClockFix(0, val & 0x3F); // get lower 6 bits
				ReInitClockTimers();

				CDC_Transmit_FS((uint8_t*)"\x01S", 2);

				ss = SERIAL_COMPLETE;
				sr = RUN_NONE;
				break;
			case SERIAL_TRANSITION:
				// process 2nd character in command for run letter
				val = Buf[byteNum];
				if(val == 'A')
				{
					sr = RUN_A;
				}
				else
				{
					ss = SERIAL_COMPLETE;
					sr = RUN_NONE;
					CDC_Transmit_FS((uint8_t*)"\xFE", 1);
					break;
				}

				byteNum++; // advance to 3rd character in command
				val = Buf[byteNum];

				byteNum++; // advance to 4th character in command

				// now we're at a uint32_t
				uint32_t tempVal;
				memcpy(&tempVal, &Buf[byteNum], 4);

				if(val == 'A') // transition to ACE
				{
					if(!AddTransition(0, TRANSITION_ACE, tempVal)) // try adding transition
					{
						// adding transition failed
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
						CDC_Transmit_FS((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(val == 'N')
				{
					if(!AddTransition(0, TRANSITION_NORMAL, tempVal)) // try adding transition
					{
						// adding transition failed
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
						CDC_Transmit_FS((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(val == 'S')
				{
					if(!AddTransition(0, TRANSITION_RESET_SOFT, tempVal)) // try adding transition
					{
						// adding transition failed
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
						CDC_Transmit_FS((uint8_t*)"\xFB", 1);
						break;
					}
				}
				else if(val == 'H')
				{
					if(!AddTransition(0, TRANSITION_RESET_HARD, tempVal)) // try adding transition
					{
						// adding transition failed
						ss = SERIAL_COMPLETE;
						sr = RUN_NONE;
						CDC_Transmit_FS((uint8_t*)"\xFB", 1);
						break;
					}
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

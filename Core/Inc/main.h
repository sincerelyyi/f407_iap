/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OC5_Pin GPIO_PIN_2
#define OC5_GPIO_Port GPIOE
#define OC4_Pin GPIO_PIN_3
#define OC4_GPIO_Port GPIOE
#define OC3_Pin GPIO_PIN_4
#define OC3_GPIO_Port GPIOE
#define OC2_Pin GPIO_PIN_5
#define OC2_GPIO_Port GPIOE
#define OC1_Pin GPIO_PIN_6
#define OC1_GPIO_Port GPIOE
#define IN8_Pin GPIO_PIN_13
#define IN8_GPIO_Port GPIOC
#define IN7_Pin GPIO_PIN_14
#define IN7_GPIO_Port GPIOC
#define IN6_Pin GPIO_PIN_15
#define IN6_GPIO_Port GPIOC
#define IN5_Pin GPIO_PIN_0
#define IN5_GPIO_Port GPIOC
#define IN4_Pin GPIO_PIN_1
#define IN4_GPIO_Port GPIOC
#define IN3_Pin GPIO_PIN_2
#define IN3_GPIO_Port GPIOC
#define SPI_DAT_Pin GPIO_PIN_3
#define SPI_DAT_GPIO_Port GPIOC
#define U4_TX_Pin GPIO_PIN_0
#define U4_TX_GPIO_Port GPIOA
#define U4_RX_Pin GPIO_PIN_1
#define U4_RX_GPIO_Port GPIOA
#define IN2_Pin GPIO_PIN_2
#define IN2_GPIO_Port GPIOA
#define OUT8_Pin GPIO_PIN_3
#define OUT8_GPIO_Port GPIOA
#define IN1_Pin GPIO_PIN_4
#define IN1_GPIO_Port GPIOA
#define SCK_6024_Pin GPIO_PIN_5
#define SCK_6024_GPIO_Port GPIOA
#define POL_6024_Pin GPIO_PIN_6
#define POL_6024_GPIO_Port GPIOA
#define DAT_6024_Pin GPIO_PIN_7
#define DAT_6024_GPIO_Port GPIOA
#define i_qe_Pin GPIO_PIN_4
#define i_qe_GPIO_Port GPIOC
#define OUT7_Pin GPIO_PIN_1
#define OUT7_GPIO_Port GPIOB
#define OUT6_Pin GPIO_PIN_9
#define OUT6_GPIO_Port GPIOE
#define OUT5_Pin GPIO_PIN_11
#define OUT5_GPIO_Port GPIOE
#define OUT4_Pin GPIO_PIN_13
#define OUT4_GPIO_Port GPIOE
#define OUT3_Pin GPIO_PIN_14
#define OUT3_GPIO_Port GPIOE
#define OUT2_Pin GPIO_PIN_10
#define OUT2_GPIO_Port GPIOB
#define OUT1_Pin GPIO_PIN_11
#define OUT1_GPIO_Port GPIOB
#define SPI_CLK_Pin GPIO_PIN_13
#define SPI_CLK_GPIO_Port GPIOB
#define VER0_Pin GPIO_PIN_10
#define VER0_GPIO_Port GPIOD
#define VER1_Pin GPIO_PIN_11
#define VER1_GPIO_Port GPIOD
#define VER2_Pin GPIO_PIN_12
#define VER2_GPIO_Port GPIOD
#define DOG_LED_Pin GPIO_PIN_8
#define DOG_LED_GPIO_Port GPIOC
#define DOG_RST_Pin GPIO_PIN_9
#define DOG_RST_GPIO_Port GPIOC
#define SCK165_Pin GPIO_PIN_3
#define SCK165_GPIO_Port GPIOB
#define DAT165_Pin GPIO_PIN_4
#define DAT165_GPIO_Port GPIOB
#define LD165_Pin GPIO_PIN_5
#define LD165_GPIO_Port GPIOB
#define OC7_Pin GPIO_PIN_0
#define OC7_GPIO_Port GPIOE
#define OC6_Pin GPIO_PIN_1
#define OC6_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

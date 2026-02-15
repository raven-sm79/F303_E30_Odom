/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f3xx_hal.h"

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
#define RST_Pin GPIO_PIN_0
#define RST_GPIO_Port GPIOB
#define DC_Pin GPIO_PIN_1
#define DC_GPIO_Port GPIOB
#define CS_Pin GPIO_PIN_10
#define CS_GPIO_Port GPIOB
#define BTN_UP_Pin GPIO_PIN_12
#define BTN_UP_GPIO_Port GPIOB
#define BTN_DN_Pin GPIO_PIN_13
#define BTN_DN_GPIO_Port GPIOB
#define BTN_SEL_Pin GPIO_PIN_14
#define BTN_SEL_GPIO_Port GPIOB
#define Speed_In_Pin GPIO_PIN_11
#define Speed_In_GPIO_Port GPIOA
#define Speed_In_EXTI_IRQn EXTI15_10_IRQn
#define Power_Hold_Pin GPIO_PIN_12
#define Power_Hold_GPIO_Port GPIOA
#define IGN_IN_Pin GPIO_PIN_15
#define IGN_IN_GPIO_Port GPIOA
#define IGN_IN_EXTI_IRQn EXTI15_10_IRQn
#define INT_Pin GPIO_PIN_9
#define INT_GPIO_Port GPIOB
#define INT_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

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
#include "stm32h5xx_hal.h"

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
#define LED0_Pin GPIO_PIN_13
#define LED0_GPIO_Port GPIOC
#define LED1_Pin GPIO_PIN_14
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_15
#define LED2_GPIO_Port GPIOC
#define ENC_EXTI2_Pin GPIO_PIN_2
#define ENC_EXTI2_GPIO_Port GPIOF
#define octo_reset_Pin GPIO_PIN_12
#define octo_reset_GPIO_Port GPIOE
#define EXTI13_TOUCH_IRQ_Pin GPIO_PIN_13
#define EXTI13_TOUCH_IRQ_GPIO_Port GPIOE
#define LCD_RS_Pin GPIO_PIN_14
#define LCD_RS_GPIO_Port GPIOE
#define LCD_RESET_Pin GPIO_PIN_15
#define LCD_RESET_GPIO_Port GPIOE
#define TOUCH_CS_Pin GPIO_PIN_10
#define TOUCH_CS_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_12
#define LCD_CS_GPIO_Port GPIOB
#define BM83_reset_Pin GPIO_PIN_10
#define BM83_reset_GPIO_Port GPIOD
#define BM83_WAKE1_Pin GPIO_PIN_14
#define BM83_WAKE1_GPIO_Port GPIOD
#define BM83_IND_Pin GPIO_PIN_15
#define BM83_IND_GPIO_Port GPIOD
#define USB_DETECT_Pin GPIO_PIN_9
#define USB_DETECT_GPIO_Port GPIOA
#define SD_CARDDET_Pin GPIO_PIN_0
#define SD_CARDDET_GPIO_Port GPIOD
#define SDMMC1_RESET_Pin GPIO_PIN_1
#define SDMMC1_RESET_GPIO_Port GPIOD
#define SAI_reset_Pin GPIO_PIN_0
#define SAI_reset_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

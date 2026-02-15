/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "st7789.h"
#include "gfx.h"
#include "text.h"
#include "ui_fonts.h"
#include "gfx.h"
#include "ui.h"
#include "buttons.h"
#include "nv.h"
#include "ds3231.h"
#include "ntc.h"
#include "ignition.h"
#include "speed.h"
#include "adc.h"
#include "vbat.h"
#include "app.h"
#include "adc_cache.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint8_t g_ds3231_irq = 0;
static uint32_t g_last_vbat_ms = 0;
volatile uint8_t g_ign_off_req = 0;

static uint8_t is_select_held_at_boot(void)
{
    // ВАЖНО: подставь реальный пин SELECT (PB14 у тебя)
    // Если кнопка тянет к GND => pressed = RESET
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET);
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_SPI1_Init();
    MX_ADC1_Init();

    IGN_Init();
    IGN_PowerHold_On();

    ADC_Cache_Init();
    VBAT_Init();

    ui_data_t g_data;
    uint16_t pulse_rem = 0;

    NV_Init();
    (void)NV_Load(&g_data, &pulse_rem);

    /* дефолты */
    memset(&g_data, 0, sizeof(g_data));
    g_data.svc_oil   = 7000;
    g_data.svc_spark = 20000;
    g_data.svc_grm   = 60000;

    SPEED_Init(pulse_rem);

    UI_Init();
    BTN_Init();

    DS3231_SetAlarmEveryMinute();
    DS3231_ClearAlarm1Flag();
    DS3231_ReadTimeDate(&g_data);


    ST7789_Init(&hspi1);
    /* дисплей init ... */
    UI_DrawStatic();
    UI_DrawAll(&g_data);

    if (is_select_held_at_boot()) {
        UI_EnterProgMode(&g_data);   // сделаем ниже
    }

  while (1)
  {

	  	  uint32_t now = HAL_GetTick();

	      ADC_Cache_Tick(now);

	      g_data.temp_c  = NTC_ReadTempC();
	      g_data.volt_mv = VBAT_Read_mV();

	      UI_DrawTopText(&g_data);

	      /* Кнопки */
	      btn_event_t ev = BTN_Poll(now);
	      if (ev.evt != BTN_EVT_NONE) {
	          UI_HandleButtonEvent(now, ev.id, ev.evt, &g_data);
	      }

	      /* 1 км */
	      if (SPEED_KmTickPending()) {
	          SPEED_ConsumeKmTick();

	          APP_OnKmTick(&g_data);          // odo++, trip++, svc--

	          UI_DrawOdoMain(&g_data);
	          UI_DrawCounters(&g_data);
	          UI_UpdateWarn(&g_data);

	          /* можно пометить “грязно”, но сохранять не каждый км */
	          UI_SetDirty();                  // если у тебя есть такой флаг
	      }

	      /* RTC minute tick */
	      if (g_ds3231_irq) {
	          g_ds3231_irq = 0;
	          DS3231_ClearAlarm1Flag();
	          DS3231_ReadTimeDate(&g_data);
	          UI_DrawTime(&g_data);
	          UI_DrawDate(&g_data);
	      }

	      /* Температура NTC раз в 250мс */
	      static uint32_t t_ntc = 0;
	      if ((now - t_ntc) >= 250u) {
	          t_ntc = now;
	          g_data.temp_c = NTC_ReadTempC();
	          UI_DrawTopText(&g_data);
	      }

	      /* VBAT раз в 1с (когда добавишь read_vbat_mv) */
	      if ((now - g_last_vbat_ms) >= 1000u) {
	          g_last_vbat_ms = now;
	          g_data.volt_mv = VBAT_Read_mV();
	          UI_DrawTopText(&g_data);
	      }

	      /* UI: авто-возврат со SVC */
	      UI_Tick(now, &g_data);

	      /* Просьба сохранить (выход из EDIT / программирование) */
	      if (UI_IsDirty()) {
	          //uint16_t rem = SPEED_GetPulseRem();
	          (void)NV_Save(&g_data, SPEED_GetPulseRem());
	          UI_ClearDirty();
	      }

	      /* Выключение зажигания */
	      if (IGN_ShutdownRequested()) {
	          IGN_ClearShutdownRequest();

	          // антидребезг/проверка реального состояния
	          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET) {
	              NV_Save(&g_data, SPEED_GetPulseRem());
	              IGN_PowerHold_Off();
	              while (1) {}
	          }
	      }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

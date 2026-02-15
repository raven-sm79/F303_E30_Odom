#include "vbat.h"
#include "adc.h"
#include "stm32f3xx_hal.h"
#include "adc_cache.h"

#ifndef VBAT_ADC_TIMEOUT_MS
#define VBAT_ADC_TIMEOUT_MS  10
#endif

// ВАЖНО: подставь реальные значения делителя!
#define VBAT_RTOP  99000.0f
#define VBAT_RBOT  15800.0f

static float vbat_k = 1.0f; // коэффициент пересчёта ADC->mV

// vbat.c
#define ADC_REF_V   3.3f

void VBAT_Init(void)
{
    // F3: лучше откалибровать перед первым чтением
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

    // Коэффициент делителя
    vbat_k = (VBAT_RTOP + VBAT_RBOT) / VBAT_RBOT;
}

uint16_t VBAT_Read_mV(void) //расчет по делителю
{
    // 1) старт
    //HAL_ADC_Start(&hadc1);

    // 2) ждать
    //if (HAL_ADC_PollForConversion(&hadc1, VBAT_ADC_TIMEOUT_MS) != HAL_OK) {
    //    HAL_ADC_Stop(&hadc1);
    //    return 0;
    //}

    // 3) считать
    uint16_t raw = ADC_GetVBAT_Raw();
    //uint32_t raw = HAL_ADC_GetValue(&hadc1);
    //HAL_ADC_Stop(&hadc1);

    // 4) ADC ref = 3.3В, 12 бит
    float v_adc = (ADC_REF_V * (float)raw) / 4095.0f;
    float v_bat = v_adc * vbat_k;

    return (uint16_t)(v_bat * 1000.0f + 0.5f);
}

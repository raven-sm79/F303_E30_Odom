#include "adc2.h"
#include "adc.h"        // hadc1
#include "stm32f3xx_hal.h"

int ADC2_Read(adc2_raw_t *out)
{
    if (!out) return -1;

    if (HAL_ADC_Start(&hadc1) != HAL_OK) return -2;

    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) { HAL_ADC_Stop(&hadc1); return -3; }
    out->ntc_raw = (uint16_t)HAL_ADC_GetValue(&hadc1);   // Rank1

    if (HAL_ADC_PollForConversion(&hadc1, 10) != HAL_OK) { HAL_ADC_Stop(&hadc1); return -4; }
    out->vbat_raw = (uint16_t)HAL_ADC_GetValue(&hadc1);  // Rank2

    (void)HAL_ADC_Stop(&hadc1);
    return 0;
}

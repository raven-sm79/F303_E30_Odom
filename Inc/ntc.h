#pragma once
#include <stdint.h>

float NTC_ReadTempC_Blocking(void);   // читает ADC, считает температуру

/* вернёт температуру в °C (целое), например 26 или -12 */
int16_t NTC_ReadTempC(void);


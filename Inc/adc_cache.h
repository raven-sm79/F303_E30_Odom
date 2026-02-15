#pragma once
#include <stdint.h>

void ADC_Cache_Init(void);
void ADC_Cache_Tick(uint32_t now_ms);  // вызывать в main loop (например, раз в 100..250мс)

uint16_t ADC_GetNTC_Raw(void);
uint16_t ADC_GetVBAT_Raw(void);

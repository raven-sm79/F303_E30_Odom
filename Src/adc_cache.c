#include "adc_cache.h"
#include "adc2.h"

static uint16_t s_ntc = 0;
static uint16_t s_vbat = 0;
static uint32_t s_next_ms = 0;

void ADC_Cache_Init(void)
{
    s_next_ms = 0;
}

void ADC_Cache_Tick(uint32_t now_ms)
{
    if ((int32_t)(now_ms - s_next_ms) < 0) return;
    s_next_ms = now_ms + 200; // 5 Гц

    adc2_raw_t r;
    if (ADC2_Read(&r) == 0) {
        s_ntc = r.ntc_raw;
        s_vbat = r.vbat_raw;
    }
}

uint16_t ADC_GetNTC_Raw(void)  { return s_ntc; }
uint16_t ADC_GetVBAT_Raw(void) { return s_vbat; }

#include "ntc.h"
#include "stm32f3xx_hal.h"
#include <math.h>
#include "adc_cache.h"

/* hadc1 из CubeMX */
extern ADC_HandleTypeDef hadc1;

/* Настройки под твой датчик (пока дефолт 10k/3950) */
#define NTC_R_FIXED   10000.0f   // резистор делителя (Ом)
#define NTC_R0        4700.0f   // NTC при 25C
#define NTC_BETA      3950.0f    // Beta
#define NTC_T0_K      298.15f    // 25C в Кельвинах

/* Усреднение */
#define NTC_SAMPLES   16u

static uint16_t adc_read_once(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t v = (uint16_t)HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return v;
}

static uint16_t adc_read_avg(void)
{
	uint16_t raw = ADC_GetNTC_Raw();
    uint32_t sum = 0;
    for (uint32_t i=0;i<NTC_SAMPLES;i++) sum += raw;
    return (uint16_t)(sum / NTC_SAMPLES);
}

/* Делитель: 3.3V -> Rfixed -> node -> NTC -> GND
   Тогда: Vnode = Vcc * Rntc / (Rfixed + Rntc)
   => Rntc = Rfixed * V / (1 - V)
   где V = adc/4095
*/
static float ntc_resistance_from_adc(uint16_t adc)
{
    if (adc == 0) return 1e9f;
    if (adc >= 4095) return 1.0f;

    float v = (float)adc / 4095.0f;
    return (NTC_R_FIXED * v) / (1.0f - v);
}

static float ntc_temp_c_from_r(float r)
{
    /* Beta formula */
    float invT = (1.0f/NTC_T0_K) + (1.0f/NTC_BETA) * logf(r / NTC_R0);
    float tK = 1.0f / invT;
    return tK - 273.15f;
}

/* лёгкий IIR-фильтр, чтобы на UI не дрожало */
int16_t NTC_ReadTempC(void)
{
    static uint8_t inited = 0;
    static float filt = 0;

    uint16_t adc = adc_read_avg();
    float r = ntc_resistance_from_adc(adc);
    float t = ntc_temp_c_from_r(r);

    if (!inited) { filt = t; inited = 1; }
    else         { filt = filt*0.85f + t*0.15f; }  // можно подкрутить

    /* округление до целого */
    if (filt >= 0) return (int16_t)(filt + 0.5f);
    else           return (int16_t)(filt - 0.5f);
}

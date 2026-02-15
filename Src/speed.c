#include "speed.h"
#include "stm32f3xx_hal.h"

static volatile uint32_t s_pulses = 0;      // остаток импульсов до 1 км (0..SPEED_PPK-1)
static volatile uint8_t  s_km_flag = 0;

/* антидребезг */
static volatile uint32_t s_last_cyc = 0;

/* выстави 500..800 мкс. Я бы начал с 500 */
#define SPEED_DEBOUNCE_US 500u

static inline void dwt_init(void)
{
    /* включаем DWT->CYCCNT */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t dwt_now(void) { return DWT->CYCCNT; }

/* перевести us в тики CPU */
static inline uint32_t us_to_cyc(uint32_t us)
{
    return us * (HAL_RCC_GetHCLKFreq() / 1000000u);
}

void SPEED_Init(uint16_t pulse_rem)
{
    dwt_init();
    s_pulses = (pulse_rem < SPEED_PPK) ? pulse_rem : 0;
    s_km_flag = 0;
    s_last_cyc = dwt_now();
}

void SPEED_OnPulseISR(void)
{
    uint32_t now = dwt_now();
    uint32_t dt  = now - s_last_cyc;               // uint32 overflow ok
    if (dt < us_to_cyc(SPEED_DEBOUNCE_US)) return;  // режем дребезг/спайки
    s_last_cyc = now;

    uint32_t p = s_pulses + 1;
    if (p >= SPEED_PPK) {
        p -= SPEED_PPK;
        s_km_flag = 1;
    }
    s_pulses = p;
}

uint8_t SPEED_KmTickPending(void) { return s_km_flag; }

void SPEED_ConsumeKmTick(void) { s_km_flag = 0; }

uint16_t SPEED_GetPulseRem(void) { return (uint16_t)s_pulses; }

void SPEED_SetPulseRem(uint16_t rem)
{
    s_pulses = (rem < SPEED_PPK) ? rem : 0;
}

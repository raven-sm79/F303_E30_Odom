#include "odo.h"
#include "speed.h"

#define PULSES_PER_KM 4838u

static uint32_t last_pulses = 0;
static uint32_t total_pulses = 0;
static uint32_t total_km = 0;

void ODO_Init(void)
{
    last_pulses = 0;
}

void ODO_Process(void)
{
    uint32_t p = SPEED_GetPulses();
    uint32_t delta = p - last_pulses;

    if (delta == 0) return;

    last_pulses = p;
    total_pulses += delta;

    while (total_pulses >= PULSES_PER_KM) {
        total_pulses -= PULSES_PER_KM;
        total_km++;
    }
}

uint32_t ODO_GetTotalKm(void)
{
    return total_km;
}

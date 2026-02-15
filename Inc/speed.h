#pragma once
#include <stdint.h>

#define SPEED_PPK 4838u  // impulses per kilometer (пока так; потом калибруем)

void     SPEED_Init(uint16_t pulse_rem);
void     SPEED_OnPulseISR(void);

uint8_t  SPEED_KmTickPending(void);
void     SPEED_ConsumeKmTick(void);

uint16_t SPEED_GetPulseRem(void);
void     SPEED_SetPulseRem(uint16_t rem);

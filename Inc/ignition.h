#pragma once
#include <stdint.h>

void IGN_Init(void);
void IGN_PowerHold_On(void);
void IGN_PowerHold_Off(void);

/* дергается из EXTI ISR */
void IGN_OnExti(void);

/* опрашивается в main */
uint8_t IGN_ShutdownRequested(void);
void IGN_ClearShutdownRequest(void);

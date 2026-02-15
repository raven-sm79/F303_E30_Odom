#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void IGN_Init(void);

/* Вызывать из HAL_GPIO_EXTI_Callback */
void IGN_OnExti(uint16_t gpio_pin);

/* true если IGN “пропал” и надо завершаться */
uint8_t IGN_ShutdownRequested(void);

/* Сбросить запрос (на случай отладки) */
void IGN_ClearShutdownRequest(void);

/* Управление удержанием питания */
void IGN_PowerHold_On(void);
void IGN_PowerHold_Off(void);

#ifdef __cplusplus
}
#endif

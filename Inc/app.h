#pragma once
#include <stdint.h>
#include "ui.h"   // тут ui_data_t

extern ui_data_t g_data;

void APP_InitDefaults(ui_data_t *d);

/* вызываем когда накопили 1 км */
void APP_OnKmTick(ui_data_t *d);

/* сбросы по индексам */
void APP_ResetTrip(ui_data_t *d, uint8_t idx);
void APP_ResetSvc (ui_data_t *d, uint8_t idx);
/* optional helpers */
uint8_t APP_SvcWarnNeeded(const ui_data_t *d, uint32_t limit_km);

#ifdef __cplusplus
}
#endif

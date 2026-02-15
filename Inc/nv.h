#pragma once
#include <stdint.h>

/* ВАЖНО:
   ui_data_t должен быть объявлен в ui.h (или другом общем заголовке).
   Тут мы его НЕ объявляем повторно, чтобы не было конфликтов типов. */
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

void NV_Init(void);

/* return 0 если нашли валидные данные, иначе -1 (caller ставит дефолты) */
int  NV_Load(ui_data_t *d, uint16_t *pulse_rem);

/* return 0 если ок */
int  NV_Save(const ui_data_t *d, uint16_t pulse_rem);

#ifdef __cplusplus
}
#endif

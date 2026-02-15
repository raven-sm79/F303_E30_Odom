#pragma once
#include <stdint.h>
#include "ui.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_TS_HH = 0,
    APP_TS_MM,
    APP_TS_DD,
    APP_TS_MO,
    APP_TS_YYYY,
    APP_TS_MAX
} app_timeset_field_t;

void APP_TimeSet_Enter(ui_data_t *d);
void APP_TimeSet_Change(ui_data_t *d, app_timeset_field_t f, int delta);

#ifdef __cplusplus
}
#endif

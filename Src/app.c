#include "app.h"

/* глобальные данные приложения */
ui_data_t g_data;

#define SVC_PRESET_OIL    7000u
#define SVC_PRESET_GRM    60000u
#define SVC_PRESET_SPARK  20000u

void APP_InitDefaults(ui_data_t *d)
{
    /* дефолты только если EEPROM пустая — вызывай перед NV_Load */
    d->svc_oil   = SVC_PRESET_OIL;
    d->svc_grm   = SVC_PRESET_GRM;
    d->svc_spark = SVC_PRESET_SPARK;
}

static inline uint32_t inc6(uint32_t v)
{
    v++;
    if (v >= 1000000u) v = 0;
    return v;
}

static inline uint32_t dec_sat(uint32_t v)
{
    if (v == 0) return 0;
    return v - 1;
}

void APP_OnKmTick(ui_data_t *d)
{
    d->odo_main = inc6(d->odo_main);

    d->trip_fuel = inc6(d->trip_fuel);
    d->trip_day  = inc6(d->trip_day);
    d->trip_ab   = inc6(d->trip_ab);

    /* сервисные “убывают” */
    d->svc_oil   = dec_sat(d->svc_oil);
    d->svc_grm   = dec_sat(d->svc_grm);
    d->svc_spark = dec_sat(d->svc_spark);
}

void APP_ResetTrip(ui_data_t *d, uint8_t idx)
{
    if (idx == 0) d->trip_fuel = 0;
    if (idx == 1) d->trip_day  = 0;
    if (idx == 2) d->trip_ab   = 0;
}

void APP_ResetSvc(ui_data_t *d, uint8_t idx)
{
    if (idx == 0) d->svc_oil   = SVC_PRESET_OIL;
    if (idx == 1) d->svc_grm   = SVC_PRESET_GRM;
    if (idx == 2) d->svc_spark = SVC_PRESET_SPARK;
}

uint8_t APP_SvcWarnNeeded(const ui_data_t *d, uint32_t limit_km)
{
    return (d->svc_oil   < limit_km) ||
           (d->svc_grm   < limit_km) ||
           (d->svc_spark < limit_km);
}

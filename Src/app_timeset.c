#include "app_timeset.h"

static uint8_t is_leap(uint16_t y)
{
    /* достаточно для 2000..2099 */
    return (uint8_t)((y % 4u) == 0u);
}

static uint8_t dim(uint16_t y, uint8_t m)
{
    static const uint8_t mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m < 1) m = 1;
    if (m > 12) m = 12;
    if (m == 2) return (uint8_t)(28u + (is_leap(y) ? 1u : 0u));
    return mdays[m - 1];
}

static uint8_t clamp_u8(uint8_t v, uint8_t lo, uint8_t hi)
{
    if (v < lo) v = lo;
    if (v > hi) v = hi;
    return v;
}

static uint16_t clamp_y(uint16_t y)
{
    /* DS3231 удобнее держать 2000..2099 */
    if (y < 2000u) y = 2000u;
    if (y > 2099u) y = 2099u;
    return y;
}

void APP_TimeSet_Enter(ui_data_t *d)
{
    /* подстрахуем, если в данных мусор */
    d->yyyy = clamp_y(d->yyyy);
    d->mo   = clamp_u8(d->mo, 1, 12);

    uint8_t maxd = dim(d->yyyy, d->mo);
    d->dd   = clamp_u8(d->dd, 1, maxd);

    d->hh   = clamp_u8(d->hh, 0, 23);
    d->mm   = clamp_u8(d->mm, 0, 59);
}

/* delta обычно +1 / -1 */
void APP_TimeSet_Change(ui_data_t *d, app_timeset_field_t f, int delta)
{
    if (delta == 0) return;

    switch (f) {
    case APP_TS_HH: {
        int v = (int)d->hh + delta;
        while (v < 0)  v += 24;
        while (v > 23) v -= 24;
        d->hh = (uint8_t)v;
    } break;

    case APP_TS_MM: {
        int v = (int)d->mm + delta;
        while (v < 0)  v += 60;
        while (v > 59) v -= 60;
        d->mm = (uint8_t)v;
    } break;

    case APP_TS_YYYY: {
        int v = (int)d->yyyy + delta;
        if (v < 2000) v = 2099;
        if (v > 2099) v = 2000;
        d->yyyy = (uint16_t)v;

        /* после смены года надо перепроверить день месяца */
        uint8_t maxd = dim(d->yyyy, d->mo);
        if (d->dd > maxd) d->dd = maxd;
    } break;

    case APP_TS_MO: {
        int v = (int)d->mo + delta;
        while (v < 1)  v += 12;
        while (v > 12) v -= 12;
        d->mo = (uint8_t)v;

        uint8_t maxd = dim(d->yyyy, d->mo);
        if (d->dd > maxd) d->dd = maxd;
    } break;

    case APP_TS_DD: {
        uint8_t maxd = dim(d->yyyy, d->mo);
        int v = (int)d->dd + delta;
        while (v < 1)    v += maxd;
        while (v > maxd) v -= maxd;
        d->dd = (uint8_t)v;
    } break;

    default:
        break;
    }
}

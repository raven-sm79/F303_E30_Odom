#include <stdio.h>
#include <string.h>
#include <stm32f3xx_hal.h>

#include "ui.h"
#include "gfx.h"
#include "ui_fonts.h"
#include "buttons.h"
#include "ui_icons.h"
#include "nv.h"   // чтобы сохранять EEPROM при выходе из EDIT
#include "app.h"
#include "app_timeset.h"
#include "ds3231.h"
#include "speed.h"

/* ===== COLORS ===== */
#define UI_FG  			0xFFFF  /* white */
#define UI_BG  			0x0000  /* black */
#define UI_RED 			0xF800
#define UI_YELLOW		0xFFE0
#define UI_ICON_COLOR	0x7BEF  // dark grey
#define UI_WARN_RED    	0xF800 // RGB565 красный

#define SVC_LIMIT_KM   100u   // меньше 100 км — красным

/* ===== Макет экрана 240x320 (книжная) ===== */

/* Верхняя строка: напряжение/температура */
#define X_VOLT    20
#define X_TEMP   170
#define Y_TOPTXT  55

/* Часы крупные */
#define X_TIME    10
#define Y_TIME    83
#define COLON_X   99
#define COLON_YOFF (0)

/* Дата */
#define X_DATE    110
#define Y_DATE   165

/* Основной пробег (сверху) */
#define X_ODO_MAIN_CENTER 120
#define Y_ODO_MAIN         10

/* Рамка блоков счетчиков */
#define BOX_X      5
#define BOX_Y    190
#define BOX_W    230
#define BOX_H    125
#define BOX_VX   120
#define BOX_HY   (BOX_Y + BOX_H/2)

/* Позиции счетчиков */
#define X_CNT_L_RIGHT   115   /* правый край левого блока */
#define X_CNT_R_RIGHT   230   /* правый край правого блока */
#define Y_CNT_0         215
#define Y_CNT_1         272

/* ==== MAIN layout: иконки слева, цифры в отдельной рамке справа ==== */

/* ==== Значок WARN ==== */
#define WARN_X  10
#define WARN_Y  10
/* ======== */

#define MAIN_ICON_COL_W   0

#define MAIN_BOX_X   (BOX_X + MAIN_ICON_COL_W)
#define MAIN_BOX_Y   (BOX_Y) //(BOX_Y + 6)
#define MAIN_BOX_W   (BOX_W - MAIN_ICON_COL_W)
#define MAIN_BOX_H   (BOX_H)

#define MAIN_ROW_H   (MAIN_BOX_H / 3)
#define MAIN_VAL_R   (MAIN_BOX_X + MAIN_BOX_W - 8)
#define MAIN_VAL_Y(i) (MAIN_BOX_Y + (i)*MAIN_ROW_H + 8)

/* ==== SVC layout: иконки слева, цифры в отдельной рамке справа ==== */

/* Ширина колонки с иконками (36px иконка + отступы) */
#define SVC_ICON_COL_W   0

#define SVC_BOX_X   (BOX_X + SVC_ICON_COL_W)
#define SVC_BOX_Y   (BOX_Y)
#define SVC_BOX_W   (BOX_W - SVC_ICON_COL_W)
#define SVC_BOX_H   (BOX_H)

/* Позиции строк внутри рамки (3 строки) */
#define SVC_ROW_H   (SVC_BOX_H / 3)
#define SVC_VAL_R   (SVC_BOX_X + SVC_BOX_W - 8)     /* правый край цифр */
#define SVC_VAL_Y(i) (SVC_BOX_Y + (i)*SVC_ROW_H + 8) /* y цифр */

#define SVC_ARROW_W  6
#define SVC_ARROW_H  18
#define SVC_ARROW_X  (SVC_BOX_X + 4)

#define ARROW_W  6
#define ARROW_H  18
#define ARROW_X  (BOX_X + 55)

/* области очистки (под твои цифры) */
#define TOPTXT_CLEAR_H   32

#define UI_SVC_TIMEOUT_MS  10000u

#define SVC_PRESET_OIL    7000u
#define SVC_PRESET_SPARK  20000u
#define SVC_PRESET_GRM    60000u

static ui_layer_t g_layer = UI_LAYER_FORWARD;

typedef struct {
    uint32_t trip_fuel;
    uint32_t trip_day;
    uint32_t trip_ab;

//    uint32_t svc_total;
    uint32_t svc_grm;
    uint32_t svc_oil;
    uint32_t svc_spark;

    uint8_t  valid;
} ui_cache_t;

static ui_cache_t g_cache = {0};

typedef struct {
    /* counters уже есть... */

    /* top text */
    char volt[8];   // "14.7V"
    char temp[10];  // "-12°C" (с 0xB0 внутри)
    char date[16];  // "22.03.2026"

    uint8_t hh, mm; // время

    uint8_t valid_top;
    uint8_t valid_time;
    uint8_t valid_date;
} ui_cache_top_t;

typedef enum {
    PROG_ODO = 0,
    PROG_OIL,
    PROG_GRM,
    PROG_SPARK,
    PROG_FIELDS
} prog_field_t;

static prog_field_t g_prog_field = PROG_ODO;

static app_timeset_field_t g_ts_field = APP_TS_HH;

static ui_cache_top_t g_top = {0};
static uint8_t  g_odo_valid = 0;
static uint32_t g_odo_last  = 0;

static ui_page_t g_page = UI_PAGE_MAIN;
static uint32_t  g_page_deadline_ms = 0; // когда надо вернуться на MAIN
static uint8_t g_warn_on = 0;
static uint8_t g_svc_drawn = 0;

/* выбор строки на сервисной странице (0..2) */
static uint8_t g_svc_sel = 0;

static ui_mode_t g_mode = UI_MODE_BROWSE;
static uint8_t g_sel_visible = 0;
static uint8_t g_sel_row = 0;
static uint8_t g_dirty = 0;   // нужно сохранить в EEPROM при выходе из EDIT

static uint32_t g_reset_exit_deadline_ms = 0;

/* ---------- helpers ---------- */

static void prog_change(ui_data_t *d, int dir, uint8_t fast)
{
    uint32_t step_odo = fast ? 1000 : 100;
    uint32_t step_svc = fast ? 1000 : 100;

    switch (g_prog_field) {
    case PROG_ODO:
        if (dir > 0) d->odo_main = (d->odo_main + step_odo) % 1000000u;
        else         d->odo_main = (d->odo_main >= step_odo) ? (d->odo_main - step_odo) : 0;
        UI_DrawOdoMain(d);
        g_dirty = 1;
        break;
    case PROG_OIL:
        if (dir > 0) d->svc_oil += step_svc; else if (d->svc_oil >= step_svc) d->svc_oil -= step_svc;
        g_svc_drawn = 0; UI_DrawCounters(d);
        g_dirty = 1;
        break;
    case PROG_GRM:
        if (dir > 0) d->svc_grm += step_svc; else if (d->svc_grm >= step_svc) d->svc_grm -= step_svc;
        g_svc_drawn = 0; UI_DrawCounters(d);
        g_dirty = 1;
        break;
    case PROG_SPARK:
        if (dir > 0) d->svc_spark += step_svc; else if (d->svc_spark >= step_svc) d->svc_spark -= step_svc;
        g_svc_drawn = 0; UI_DrawCounters(d);
        g_dirty = 1;
        break;
    default: break;
    }
}

static void prog_next_field(ui_data_t *d)
{
    // убрать старый фокус
    // ODO: стереть рамку (перерисовать фон/линию) — проще просто перерисовать UI_DrawOdoMain + линии.
    // SVC: стрелку перерисовать.
    if (g_prog_field == PROG_ODO) {
        // стереть рамку вокруг одометра
        GFX_DrawRect(10, Y_ODO_MAIN-2, 220, 42, UI_BG);
        UI_DrawOdoMain(d);
    }

    g_prog_field = (prog_field_t)((g_prog_field + 1) % PROG_FIELDS);

    if (g_prog_field == PROG_ODO) {
        // фокус на одометре
        GFX_DrawRect(10, Y_ODO_MAIN-2, 220, 42, UI_YELLOW);
    } else {
        // фокус на строке сервиса
        uint8_t row = (uint8_t)(g_prog_field - PROG_OIL); // 0..2
        UI_SetSelRow(row);
        UI_SetSelVisible(1);
    }
}

uint8_t UI_InProgMode(void) { return (g_mode == UI_MODE_PROG); }

static void prog_draw_focus(prog_field_t f, uint16_t color)
{
    // Тут проще всего: использовать твою стрелку (draw_arrow_row) и страницу SVC
    // Сделаем: в PROG показываем SVC-страницу, но в ней 4 строки нет.
    // Поэтому проще:
    //  - оставить 3 строки на экране (как сейчас)
    //  - ODO_MAIN редактировать вверху (6x36) отдельным фокусом
    //  - сервисные интервалы редактировать на SVC странице (3 строки) стрелкой

    // Минимально:
    // ODO: подсветить одометр рамкой
    // SVC: стрелкой строки 0..2
}

static inline uint16_t svc_color(uint32_t remaining_km)
{
    return (remaining_km < SVC_LIMIT_KM) ? UI_WARN_RED : UI_FG;
}
static void timeset_draw_focus(app_timeset_field_t f, uint16_t color)
{
    /* Небольшие поля вокруг цифр */
    const int pad = 2;

    /* ---- TIME "HH:MM" ----
       У тебя HH начинается в X_TIME, MM — в X_TIME + 3*a75 (как в draw_time())
    */
    const int a75 = UIF_Adv75();
    const int h75 = UIF_H75();

    const int hh_x = X_TIME + 0 * a75;
    const int mm_x = X_TIME + 3 * a75;
    const int t_y  = Y_TIME;

    /* ---- DATE "DD.MM.YYYY" ----
       В твоём draw_date() ты делаешь:
         x = X_DATE - 4*a12
       Значит:
         DD  = x + 0*a12
         MM  = x + 3*a12
         YYYY= x + 6*a12
    */
    const int a12 = UIF_Adv12();
    const int h12 = UIF_H12();
    const int date_x0 = X_DATE - 4 * a12;
    const int d_y = Y_DATE;

    switch (f) {
    case APP_TS_HH:
        GFX_DrawRect(hh_x - pad, t_y - pad, (2*a75 - 1) + 2*pad, h75 + 2*pad, color);
        break;

    case APP_TS_MM:
        GFX_DrawRect(mm_x - pad, t_y - pad, (2*a75 - 1) + 2*pad, h75 + 2*pad, color);
        break;

    case APP_TS_DD:
        GFX_DrawRect((date_x0 + 0*a12) - pad, d_y - pad, (2*a12 - 1) + 2*pad, h12 + 2*pad, color);
        break;

    case APP_TS_MO:
        GFX_DrawRect((date_x0 + 3*a12) - pad, d_y - pad, (2*a12 - 1) + 2*pad, h12 + 2*pad, color);
        break;

    case APP_TS_YYYY:
        GFX_DrawRect((date_x0 + 6*a12) - pad, d_y - pad, (4*a12 - 1) + 2*pad, h12 + 2*pad, color);
        break;

    default:
        break;
    }
}
static inline uint8_t svc_warn_needed(const ui_data_t *d)
{
    return (d->svc_oil   < SVC_LIMIT_KM) ||
           (d->svc_grm   < SVC_LIMIT_KM) ||
           (d->svc_spark < SVC_LIMIT_KM);
}

static void ui_warn_update(const ui_data_t *d)
{
    uint8_t need = svc_warn_needed(d);
    if (need == g_warn_on) return;

    if (need) {
        /* красная иконка WARN */
        UI_DrawIcon36(WARN_X, WARN_Y, ICON_ERROR, UI_WARN_RED, UI_BG);
    } else {
        /* стереть область */
        GFX_FillRect(WARN_X, WARN_Y, UI_ICON_W, UI_ICON_H, UI_BG);
    }
    g_warn_on = need;
}

static void draw_arrow_row(uint8_t row, uint16_t color)
{
    /* 3 строки по BOX_H */
    int row_h = BOX_H / 3;
    int y = BOX_Y + row * row_h + (row_h - ARROW_H)/2;
    GFX_FillRect(ARROW_X, y, ARROW_W, ARROW_H, color);
}

static void redraw_string12(int x, int y, const char *old_s, const char *new_s)
{
    if (old_s && old_s[0]) {
        /* стереть старое */
        UIF_DrawString12(x, y, old_s, UI_BG, UI_BG);
    }
    /* нарисовать новое */
    if (new_s && new_s[0]) {
        UIF_DrawString12(x, y, new_s, UI_FG, UI_BG);
    }
}

static void redraw_digit75(int x, int y, uint8_t old_d, uint8_t new_d)
{
    if (old_d == new_d) return;
    UIF_DrawDigit75(x, y, old_d, UI_BG, UI_BG);   // стереть
    UIF_DrawDigit75(x, y, new_d, UI_FG, UI_BG);   // нарисовать
}

static void redraw_uint27_right6(int right_x, int y, uint32_t oldv, uint32_t newv)
{
    if (oldv == newv) return;
    UIF_UpdateNumber6_Right27(right_x, y, oldv, newv, UI_YELLOW, UI_BG);
}

static void draw_voltage(uint16_t mv)
{
    uint16_t v  = mv / 1000;
    uint16_t d1 = (mv % 1000) / 100;

    char buf[8];
    snprintf(buf, sizeof(buf), "%u.%uV", (unsigned)v, (unsigned)d1);

    if (!g_top.valid_top || strcmp(g_top.volt, buf) != 0) {
        redraw_string12(X_VOLT, Y_TOPTXT, g_top.valid_top ? g_top.volt : "", buf);
        strncpy(g_top.volt, buf, sizeof(g_top.volt));
        g_top.volt[sizeof(g_top.volt) - 1] = 0;
    }

    g_top.valid_top = 1;
}

static void draw_temp(int16_t t)
{
    char buf[10];
    int neg = (t < 0);
    if (neg) t = -t;

    /* Собираем "-12°C" в одну строку, чтобы стирать/рисовать одинаково */
    if (neg) snprintf(buf, sizeof(buf), "-%d%cC", (int)t, (char)0xB0);
    else     snprintf(buf, sizeof(buf),  "%d%cC", (int)t, (char)0xB0);

    if (!g_top.valid_top || strcmp(g_top.temp, buf) != 0) {
        redraw_string12(X_TEMP, Y_TOPTXT, g_top.valid_top ? g_top.temp : "", buf);
        strncpy(g_top.temp, buf, sizeof(g_top.temp));
        g_top.temp[sizeof(g_top.temp) - 1] = 0;
    }

    g_top.valid_top = 1;
}

static void draw_time(uint8_t hh, uint8_t mm)
{
    int a75 = UIF_Adv75();
    int x = X_TIME;
    int y = Y_TIME;

    /* Первый запуск — просто нарисуем всё */
    if (!g_top.valid_time) {
        UIF_DrawDigit75(x + 0*a75, y, hh/10, UI_FG, UI_BG);
        UIF_DrawDigit75(x + 1*a75, y, hh%10, UI_FG, UI_BG);
        UIF_DrawPunct75(COLON_X, y + COLON_YOFF, ':', UI_FG, UI_BG);
        UIF_DrawDigit75(x + 3*a75, y, mm/10, UI_FG, UI_BG);
        UIF_DrawDigit75(x + 4*a75, y, mm%10, UI_FG, UI_BG);

        g_top.hh = hh;
        g_top.mm = mm;
        g_top.valid_time = 1;
        return;
    }

    /* Стираем и рисуем только те цифры, которые поменялись */
    redraw_digit75(x + 0*a75, y, g_top.hh/10, hh/10);
    redraw_digit75(x + 1*a75, y, g_top.hh%10, hh%10);
    redraw_digit75(x + 3*a75, y, g_top.mm/10, mm/10);
    redraw_digit75(x + 4*a75, y, g_top.mm%10, mm%10);

    /* двоеточие не трогаем (можно потом сделать мигание отдельной функцией) */

    g_top.hh = hh;
    g_top.mm = mm;
}

static void draw_date(uint8_t dd, uint8_t mo, uint16_t yyyy)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%02u.%02u.%04u", dd, mo, (unsigned)yyyy);

    /* как у тебя: сдвиг влево от X_DATE */
    int a12 = UIF_Adv12();
    int x = X_DATE - 4 * a12;

    if (!g_top.valid_date || strcmp(g_top.date, buf) != 0) {
        redraw_string12(x, Y_DATE, g_top.valid_date ? g_top.date : "", buf);
        strncpy(g_top.date, buf, sizeof(g_top.date));
        g_top.date[sizeof(g_top.date) - 1] = 0;
        g_top.valid_date = 1;
    }
}

static void draw_odo_main(uint32_t val)
{
    if (val > 999999) val %= 1000000;

    if (!g_odo_valid) {
        UIF_DrawNumber6_Center36_Fixed(X_ODO_MAIN_CENTER, Y_ODO_MAIN, val, UI_YELLOW, UI_BG);
        g_odo_last = val;
        g_odo_valid = 1;
        return;
    }

    if (g_odo_last == val) return;

    /* стереть старое ровно по пикселям одометра */
    UIF_DrawNumber6_Center36_Fixed(X_ODO_MAIN_CENTER, Y_ODO_MAIN, g_odo_last, UI_BG, UI_BG);

    /* нарисовать новое */
    UIF_DrawNumber6_Center36_Fixed(X_ODO_MAIN_CENTER, Y_ODO_MAIN, val, UI_YELLOW, UI_BG);

    g_odo_last = val;
}

static void draw_page_main_static_icons(void)
{
    uint16_t ic = UI_ICON_COLOR;

    int x = BOX_X + 10;
    int y0 = 192; //BOX_Y + 10;
    int y1 = 234; //BOX_Y + 10 + 38;
    int y2 = 276; //BOX_Y + 10 + 76;

    UI_DrawIcon36(x, y0, ICON_BENZ, ic, UI_BG);
    UI_DrawIcon36(x, y1, ICON_SUT,  ic, UI_BG);
    UI_DrawIcon36(x, y2, ICON_AB,   ic, UI_BG);

    /* разделители строк (2 линии) */
    GFX_DrawHLine(MAIN_BOX_X, MAIN_BOX_Y + MAIN_ROW_H,     MAIN_BOX_W, UI_FG);
    GFX_DrawHLine(MAIN_BOX_X, MAIN_BOX_Y + 2*MAIN_ROW_H,   MAIN_BOX_W, UI_FG);
}


static void draw_page_svc_static_icons(void)
{
    uint16_t ic = UI_ICON_COLOR;

    int x = BOX_X + 10;
    int y0 = 192; //BOX_Y + 10;
    int y1 = 234; //BOX_Y + 10 + 38;
    int y2 = 276; //BOX_Y + 10 + 76;

    UI_DrawIcon36(x, y0, ICON_OIL,   ic, UI_BG);
    UI_DrawIcon36(x, y1, ICON_GRM,   ic, UI_BG);
    UI_DrawIcon36(x, y2, ICON_SPARK, ic, UI_BG);

    /* разделители строк (2 линии) */
    GFX_DrawHLine(SVC_BOX_X, SVC_BOX_Y + SVC_ROW_H,     SVC_BOX_W, UI_FG);
    GFX_DrawHLine(SVC_BOX_X, SVC_BOX_Y + 2*SVC_ROW_H,   SVC_BOX_W, UI_FG);
}

static void draw_page_main_values(const ui_data_t *d)
{
    redraw_uint27_right6(MAIN_VAL_R, MAIN_VAL_Y(0), g_cache.trip_fuel, d->trip_fuel);
    redraw_uint27_right6(MAIN_VAL_R, MAIN_VAL_Y(1), g_cache.trip_day,  d->trip_day);
    redraw_uint27_right6(MAIN_VAL_R, MAIN_VAL_Y(2), g_cache.trip_ab,   d->trip_ab);

    g_cache.trip_fuel = d->trip_fuel;
    g_cache.trip_day  = d->trip_day;
    g_cache.trip_ab   = d->trip_ab;
}

static void draw_page_svc_values_once(const ui_data_t *d)
{
    /* разделители строк снова (после fill) */
    GFX_DrawHLine(SVC_BOX_X, SVC_BOX_Y + SVC_ROW_H,     SVC_BOX_W, UI_FG);
    GFX_DrawHLine(SVC_BOX_X, SVC_BOX_Y + 2*SVC_ROW_H,   SVC_BOX_W, UI_FG);

    /* цифры (цвет зависит от лимита) */
    UIF_DrawNumber6_Right27_Fixed(SVC_VAL_R, SVC_VAL_Y(0), d->svc_oil, UI_BG, UI_BG);
    UIF_DrawUInt_Right27(SVC_VAL_R, SVC_VAL_Y(0), d->svc_oil,   svc_color(d->svc_oil),   UI_BG);
    UIF_DrawNumber6_Right27_Fixed(SVC_VAL_R, SVC_VAL_Y(1), d->svc_grm, UI_BG, UI_BG);
    UIF_DrawUInt_Right27(SVC_VAL_R, SVC_VAL_Y(1), d->svc_grm,   svc_color(d->svc_grm),   UI_BG);
    UIF_DrawNumber6_Right27_Fixed(SVC_VAL_R, SVC_VAL_Y(2), d->svc_spark, UI_BG, UI_BG);
    UIF_DrawUInt_Right27(SVC_VAL_R, SVC_VAL_Y(2), d->svc_spark, svc_color(d->svc_spark), UI_BG);

    g_svc_drawn = 1;
}

static ui_page_t page_next(ui_page_t p)
{
    // масштабируемо: если потом добавишь страницы — просто расширишь enum
    if (p >= UI_PAGE_SVC) return UI_PAGE_MAIN;
    return (ui_page_t)(p + 1);
}

static ui_page_t page_prev(ui_page_t p)
{
    if (p == UI_PAGE_MAIN) return UI_PAGE_SVC;
    return (ui_page_t)(p - 1);
}

/* ---------- public ---------- */

void UI_EnterProgMode(ui_data_t *d)
{
    g_mode = UI_MODE_PROG;

    // Покажем сервисную страницу (там 3 строки — масло/грм/свечи)
    UI_SetPage(UI_PAGE_SVC);
    g_svc_drawn = 0;
    UI_DrawCounters(d);

    // И одометр сверху тоже оставляем видимым
    UI_DrawOdoMain(d);

    // По умолчанию редактируем ODO
    g_prog_field = PROG_ODO;

    // Нарисуй какой-то индикатор режима (например, маленькую иконку "SET" или рамку)
    // Минимум: прямоугольник вокруг одометра
    GFX_DrawRect(10, Y_ODO_MAIN-2, 220, 42, UI_YELLOW);
}

uint8_t UI_IsDirty(void)
{
    return g_dirty;
}

void UI_ClearDirty(void)
{
    g_dirty = 0;
}

void UI_SetDirty(void)
{
    g_dirty = 1;
}

void UI_UpdateWarn(const ui_data_t *d) { ui_warn_update(d); }

void UI_Init(void)
{
    g_layer = UI_LAYER_FORWARD;
}

void UI_DrawStatic(void)
{

	GFX_FillScreen(UI_BG);
	//GFX_FillScreen(UI_BG);0x7BEF

    /* общая рамка нижнего блока */
    GFX_DrawRect(BOX_X, BOX_Y, BOX_W, BOX_H, UI_FG);

    if (g_page == UI_PAGE_MAIN) draw_page_main_static_icons();
    else                        draw_page_svc_static_icons();

    /* сброс кэшей после полного фона */
    memset(&g_cache, 0, sizeof(g_cache));
    memset(&g_top, 0, sizeof(g_top));
    g_odo_valid = 0;
}

void UI_SetCountersLayer(ui_layer_t layer)
{
    if (layer > UI_LAYER_REVERSE) layer = UI_LAYER_FORWARD;
    g_layer = layer;
}

ui_layer_t UI_GetCountersLayer(void)
{
    return g_layer;
}

void UI_DrawTopText(const ui_data_t *d)
{
    draw_voltage(d->volt_mv);
    draw_temp(d->temp_c);
}

void UI_DrawTime(const ui_data_t *d)
{
    draw_time(d->hh, d->mm);
}

void UI_DrawDate(const ui_data_t *d)
{
    draw_date(d->dd, d->mo, d->yyyy);
}

void UI_DrawOdoMain(const ui_data_t *d)
{
    draw_odo_main(d->odo_main);
}

void UI_DrawCounters(const ui_data_t *d)
{
    if (!g_cache.valid) {
        g_cache.valid = 1;
        g_cache.trip_fuel = ~d->trip_fuel;
        g_cache.trip_day  = ~d->trip_day;
        g_cache.trip_ab   = ~d->trip_ab;
    }

    if (g_page == UI_PAGE_MAIN) {
        draw_page_main_values(d);  /* динамика */
    } else {
        if (!g_svc_drawn) {
            draw_page_svc_values_once(d); /* статика */
        }
    }
}

void UI_DrawAll(const ui_data_t *d)
{
    UI_DrawTopText(d);
    UI_DrawTime(d);
    UI_DrawDate(d);
    UI_DrawOdoMain(d);
    UI_DrawCounters(d);
    ui_warn_update(d);
}

void UI_SetPage(ui_page_t page)
{
    if (page > UI_PAGE_SVC) page = UI_PAGE_MAIN;
    if (page == g_page) return;

    g_page = page;

    /* Перерисуем низ целиком (фон + рамка + линии + иконки),
       чтобы сменить набор иконок и убрать “чужие” элементы */
    /* Низовой фон: только внутри BOX */
    GFX_FillRect(BOX_X+1, BOX_Y+1, BOX_W-2, BOX_H-2, UI_BG);
    GFX_DrawRect(BOX_X, BOX_Y, BOX_W, BOX_H, UI_FG);

    if (g_page == UI_PAGE_MAIN) draw_page_main_static_icons();
    else                        draw_page_svc_static_icons();

    /* Сброс кэша цифр — чтобы значения гарантированно нарисовались */
    g_cache.valid = 0;

    if (g_page == UI_PAGE_SVC) {
        g_svc_drawn = 0;
        g_svc_sel = 0;
    }
    else g_page_deadline_ms = 0;
}

ui_page_t UI_GetPage(void)
{
    return g_page;
}

void UI_OnUserActivity(uint32_t now_ms)
{
    /* Любая активность на сервисной странице продлевает её жизнь */
    if (g_page == UI_PAGE_SVC) {
        g_page_deadline_ms = now_ms + UI_SVC_TIMEOUT_MS;
    }
}

void UI_Tick(uint32_t now_ms, const ui_data_t *d)
{
    if (g_page == UI_PAGE_SVC ) {
        if (g_page_deadline_ms == 0) {
            g_page_deadline_ms = now_ms + UI_SVC_TIMEOUT_MS;
        } else if ((int32_t)(now_ms - g_page_deadline_ms) >= 0) {
            /* Время вышло — возвращаемся на MAIN и перерисовываем только нижний блок */
        	UI_SetPage(UI_PAGE_MAIN);
            g_page_deadline_ms = 0;
            UI_DrawCounters(d);
        }
    }

    if (g_reset_exit_deadline_ms) {
        if ((int32_t)(now_ms - g_reset_exit_deadline_ms) >= 0) {
            g_reset_exit_deadline_ms = 0;

            // выйти в MAIN и в BROWSE
            UI_SetSelVisible(0);
            g_mode = UI_MODE_BROWSE;

            UI_SetPage(UI_PAGE_MAIN);
            // сохранить
            g_dirty = 1;
            UI_DrawCounters(d);
        }
    }
}

void UI_SetSelVisible(uint8_t on)
{
    if (on == g_sel_visible) return;
    g_sel_visible = on;

    if (!g_sel_visible) {
        // стереть стрелку
    	draw_arrow_row(g_sel_row, UI_BG);
    } else {
        // нарисовать стрелку
    	draw_arrow_row(g_sel_row, UI_FG);
    }
}

void UI_SetSelRow(uint8_t row)
{
    if (row > 2) row = 2;
    if (row == g_sel_row) return;

    if (g_sel_visible) draw_arrow_row(g_sel_row, UI_BG);
    g_sel_row = row;
    if (g_sel_visible) draw_arrow_row(g_sel_row, UI_FG);
}

uint8_t UI_GetSelRow(void) { return g_sel_row; }

void UI_ResetSelectedCounter(ui_data_t *d)
{

    if (g_page == UI_PAGE_MAIN) {
        APP_ResetTrip(d, g_sel_row);
        g_cache.valid = 0;
        UI_DrawCounters(d);
    } else {
        APP_ResetSvc(d, g_sel_row);
        g_svc_drawn = 0;
        UI_DrawCounters(d);
    }

    g_dirty = 1;
    UI_UpdateWarn(d);
}

void UI_HandleButtonEvent(uint32_t now_ms, btn_id_t id, btn_evt_t evt, ui_data_t *d)
{
    (void)now_ms;

    // PROG MODE
    if (g_mode == UI_MODE_PROG) {

        if (evt == BTN_EVT_HOLD_2S && id == BTN_SEL) {
            // Выход из PROG с сохранением
            UI_SetSelVisible(0);
            if (g_dirty) {
                (void)NV_Save(d, SPEED_GetPulseRem());
                g_dirty = 0;
            }
            g_mode = UI_MODE_BROWSE;
            UI_SetPage(UI_PAGE_MAIN);
            return;
        }

        if (evt == BTN_EVT_PRESS) {
            if (id == BTN_UP)  { prog_change(d, +1, 0); return; }
            if (id == BTN_DN)  { prog_change(d, -1, 0); return; }
            if (id == BTN_SEL) { prog_next_field(d);     return; }
        }

        // можно добавить FAST: если evt == BTN_EVT_REPEAT или HOLD_2S на UP/DN -> fast=1
        return;
    }

    if (evt == BTN_EVT_NONE) return;

    // 1) HOLD2S на SELECT — сброс выбранного (только в EDIT)
    if (id == BTN_SEL && evt == BTN_EVT_HOLD_2S) {

        if (g_mode == UI_MODE_TIMESET) {
            /* стереть фокус */
            timeset_draw_focus(g_ts_field, UI_BG);

            (void)DS3231_WriteTimeDate(d);
            g_mode = UI_MODE_BROWSE;
            return;
        }

    	if (g_mode == UI_MODE_EDIT) {
            UI_ResetSelectedCounter(d);
            g_reset_exit_deadline_ms = HAL_GetTick() + 10000u;
            return;
    	}

        if (g_mode == UI_MODE_BROWSE && g_page == UI_PAGE_MAIN) {
            g_mode = UI_MODE_TIMESET;
            g_ts_field = APP_TS_HH;

            APP_TimeSet_Enter(d);

            /* перерисуем время/дату чтобы всё было консистентно */
            UI_DrawTime(d);
            UI_DrawDate(d);

            /* фокус */
            timeset_draw_focus(g_ts_field, UI_YELLOW);
            return;
        }
//        return;
    }

    /* TIMESET mode */
    if (g_mode == UI_MODE_TIMESET)
    {

        if (evt != BTN_EVT_PRESS) return;

        /* перед сменой поля — стереть старую рамку */
        timeset_draw_focus(g_ts_field, UI_BG);

        if (id == BTN_SEL) {
            /* следующий параметр */
            g_ts_field = (app_timeset_field_t)((g_ts_field + 1) % APP_TS_MAX);
        } else if (id == BTN_UP) {
            APP_TimeSet_Change(d, g_ts_field, +1);
        } else if (id == BTN_DN) {
            APP_TimeSet_Change(d, g_ts_field, -1);
        }

        /* перерисовать нужные части */
        UI_DrawTime(d);
        UI_DrawDate(d);

        /* нарисовать новую рамку */
        timeset_draw_focus(g_ts_field, UI_YELLOW);
        return;
    }

    // 2) Короткие нажатия
    if (evt != BTN_EVT_PRESS) return;

    if (g_mode == UI_MODE_BROWSE) {
        // BROWSE: UP/DN листают страницы, SELECT фиксирует текущую
        if (id == BTN_UP) {
            UI_SetPage(page_prev(g_page));
            UI_DrawCounters(d);
        } else if (id == BTN_DN) {
            UI_SetPage(page_next(g_page));
            UI_DrawCounters(d);
        } else if (id == BTN_SEL) {
            g_mode = UI_MODE_EDIT;
            g_sel_row = 0;
            UI_SetSelRow(0);
            UI_SetSelVisible(1);

            // на SVC включаем таймер авто-возврата только при активности
            g_reset_exit_deadline_ms = HAL_GetTick() + 10000u;
            //UI_OnUserActivity(HAL_GetTick());
        }
        return;
    }

    // EDIT mode
    if (g_mode == UI_MODE_EDIT) {

        if (id == BTN_UP) {
            if (g_sel_row > 0) UI_SetSelRow(g_sel_row - 1);
            //UI_OnUserActivity(HAL_GetTick());
        } else if (id == BTN_DN) {
            if (g_sel_row < 2) UI_SetSelRow(g_sel_row + 1);
            //UI_OnUserActivity(HAL_GetTick());
        } else if (id == BTN_SEL) {
            // выход из фиксации: убрали стрелку и сохранили, если грязно
            UI_SetSelVisible(0);
            g_mode = UI_MODE_BROWSE;

            if (g_dirty) {
            	(void)NV_Save(d, SPEED_GetPulseRem());
                g_dirty = 1;
            }
        }

        return;
    }



}


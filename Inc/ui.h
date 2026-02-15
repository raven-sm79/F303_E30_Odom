#ifndef INC_UI_H_
#define INC_UI_H_

#include <stdint.h>
#include "buttons.h"

typedef struct {
    uint16_t volt_mv;   // если у тебя mV, сделай uint16_t/uint32_t как удобно
    int16_t  temp_c;

    uint8_t  hh, mm;
    uint8_t  dd, mo;
    uint16_t yyyy;

    uint32_t odo_main;

    uint32_t trip_fuel;
    uint32_t trip_day;
    uint32_t trip_ab;

    uint32_t svc_oil;
    uint32_t svc_grm;
    uint32_t svc_spark;
} ui_data_t;

typedef enum {
    UI_LAYER_FORWARD = 0,
    UI_LAYER_REVERSE = 1
} ui_layer_t;

/* Данные для одного слоя счётчиков (3 окна) */
typedef struct {
    uint32_t c0;
    uint32_t c1;
    uint32_t c2;
} ui_counters_t;

/* Все данные для UI */
/* Страница UI (у тебя уже есть, оставь как есть) */
typedef enum {
    UI_PAGE_MAIN = 0,
    UI_PAGE_SVC  = 1,
} ui_page_t;

/* Режим UI (если используешь) */
typedef enum {
    UI_MODE_BROWSE = 0,
    UI_MODE_EDIT   = 1,
	UI_MODE_TIMESET = 2,
	UI_MODE_PROG   = 3,  /* удержание Select при старте */
} ui_mode_t;

/* ===== Главные данные приборки, которые сохраняем в EEPROM =====
typedef struct
{
    // Верхняя часть UI
    uint16_t volt_mv;   // можно не сохранять, но пусть будет в структуре данных для отрисовки
    int16_t  temp_c;    // можно не сохранять
    uint8_t  hh, mm;
    uint8_t  dd, mo;
    uint16_t yyyy;

    // Одометр общий (растёт)
    uint32_t odo_main;      // 0..999999 (UI ограничит)

    // TRIP (растут, сброс -> 0)
    uint32_t trip_fuel;     // 0..999999
    uint32_t trip_day;      // 0..999999
    uint32_t trip_ab;       // 0..999999

    // SVC остатки (убывают, 0..999999), WARN если < 100
    uint32_t svc_oil;
    uint32_t svc_grm;
    uint32_t svc_spark;

    // SVC интервалы (куда сбрасываем по удержанию Select)
    uint32_t svc_oil_int;
    uint32_t svc_grm_int;
    uint32_t svc_spark_int;

} ui_data_t;
*/

void UI_EnterProgMode(ui_data_t *d);
uint8_t UI_InProgMode(void);

uint8_t UI_IsDirty(void);
void    UI_ClearDirty(void);
void UI_SetDirty(void);

/* В UI должны быть функции для стрелки и смены страницы */
void UI_SetPage(ui_page_t page);
ui_page_t UI_GetPage(void);

/* стрелка выбора (рисуем/стираем fillrect), строка 0..2 */
void UI_SetSelVisible(uint8_t on);
void UI_SetSelRow(uint8_t row);
uint8_t UI_GetSelRow(void);

/* сервисные пресеты (остаток после "сброса") */
void UI_ResetSelectedCounter(ui_data_t *d);

/* Обработчик кнопок (внешний вход) */
void UI_HandleButtonEvent(uint32_t now_ms, btn_id_t id, btn_evt_t evt, ui_data_t *d);

void UI_SetPage(ui_page_t page);                 // вручную (кнопки позже)
ui_page_t UI_GetPage(void);

void UI_OnUserActivity(uint32_t now_ms);         // дергать при нажатии кнопок
void UI_Tick(uint32_t now_ms, const ui_data_t *d); // дергать в main while(1)

/* API */
void UI_Init(void);
void UI_DrawStatic(void);
void UI_UpdateWarn(const ui_data_t *d);

void UI_SetCountersLayer(ui_layer_t layer);
ui_layer_t UI_GetCountersLayer(void);

/* Обновления по частям */
void UI_DrawTopText(const ui_data_t *d);     // volt + temp
void UI_DrawTime(const ui_data_t *d);
void UI_DrawDate(const ui_data_t *d);
void UI_DrawOdoMain(const ui_data_t *d);
void UI_DrawCounters(const ui_data_t *d);

/* Полная перерисовка динамики (статик отдельно) */
void UI_DrawAll(const ui_data_t *d);

/* Тест */
void UI_DrawStaticTest(void);

#endif

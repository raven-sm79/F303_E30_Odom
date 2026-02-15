#pragma once
#include <stdint.h>

typedef enum { BTN_UP=0, BTN_DN, BTN_SEL } btn_id_t;

typedef enum {
    BTN_EVT_NONE = 0,
    BTN_EVT_PRESS,     // короткое нажатие (генерим на отпускании, если не было hold)
    BTN_EVT_HOLD_2S     // удержание >= 2с (однократно)
} btn_evt_t;

typedef struct {
    btn_id_t id;
    btn_evt_t evt;
} btn_event_t;

void BTN_Init(void);
btn_event_t BTN_Poll(uint32_t now_ms);

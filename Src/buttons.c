#include "buttons.h"
#include "stm32f3xx_hal.h"

/* PB12=UP, PB13=DOWN, PB14=SELECT */
#define BTN_UP_GPIO   GPIOB
#define BTN_UP_PIN    GPIO_PIN_12
#define BTN_DN_GPIO   GPIOB
#define BTN_DN_PIN    GPIO_PIN_13
#define BTN_SEL_GPIO  GPIOB
#define BTN_SEL_PIN   GPIO_PIN_14

#define DEBOUNCE_MS   30u
#define HOLD_MS       2000u

typedef struct {
    uint8_t stable;        // 1 up, 0 pressed
    uint8_t last_raw;
    uint32_t t_last_change;

    uint8_t pressed;       // 1 если сейчас "в нажатом"
    uint8_t hold_fired;
    uint32_t t_press;
} btn_state_t;

static btn_state_t s_btn[3];

static inline uint8_t read_raw(btn_id_t id)
{
    GPIO_TypeDef *p = 0; uint16_t pin = 0;
    switch (id) {
        case BTN_UP:  p = BTN_UP_GPIO;  pin = BTN_UP_PIN;  break;
        case BTN_DN:  p = BTN_DN_GPIO;  pin = BTN_DN_PIN;  break;
        case BTN_SEL: p = BTN_SEL_GPIO; pin = BTN_SEL_PIN; break;
        default: return 1;
    }
    return (uint8_t)HAL_GPIO_ReadPin(p, pin); // 1=up, 0=pressed
}

void BTN_Init(void)
{
    for (int i=0;i<3;i++){
        uint8_t r = read_raw((btn_id_t)i);
        s_btn[i] = (btn_state_t){
            .stable = r, .last_raw = r, .t_last_change = 0,
            .pressed = (r==0), .hold_fired = 0, .t_press = 0
        };
    }
}

btn_event_t BTN_Poll(uint32_t now_ms)
{
    btn_event_t ev = { .evt = BTN_EVT_NONE, .id = BTN_UP };

    for (int i=0;i<3;i++){
        btn_id_t id = (btn_id_t)i;
        uint8_t raw = read_raw(id);

        if (raw != s_btn[i].last_raw) {
            s_btn[i].last_raw = raw;
            s_btn[i].t_last_change = now_ms;
        }

        if ((now_ms - s_btn[i].t_last_change) >= DEBOUNCE_MS) {
            if (raw != s_btn[i].stable) {
                s_btn[i].stable = raw;

                if (raw == 0) {
                    // press start
                    s_btn[i].pressed = 1;
                    s_btn[i].hold_fired = 0;
                    s_btn[i].t_press = now_ms;
                } else {
                    // release
                    uint8_t was_pressed = s_btn[i].pressed;
                    s_btn[i].pressed = 0;

                    // короткий PRESS только если не было HOLD
                    if (was_pressed && !s_btn[i].hold_fired) {
                        ev.id = id;
                        ev.evt = BTN_EVT_PRESS;
                        return ev;
                    }
                }
            }
        }

        // HOLD проверяем по стабильному состоянию "нажата"
        if (s_btn[i].pressed && !s_btn[i].hold_fired) {
            if ((now_ms - s_btn[i].t_press) >= HOLD_MS) {
                s_btn[i].hold_fired = 1;
                ev.id = id;
                ev.evt = BTN_EVT_HOLD_2S;
                return ev;
            }
        }
    }
    return ev;
}

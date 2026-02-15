#include "ignition.h"
#include "stm32f3xx_hal.h"

/* ВАЖНО: эти define’ы должны совпасть с CubeMX именами */
#define IGN_GPIO_Port      GPIOA
#define IGN_Pin            GPIO_PIN_15

#define HOLD_GPIO_Port     GPIOA
#define HOLD_Pin           GPIO_PIN_12

static volatile uint8_t s_shutdown_req = 0;

void IGN_Init(void)
{
    s_shutdown_req = 0;
}

/* Держим питание */
void IGN_PowerHold_On(void)
{
    HAL_GPIO_WritePin(HOLD_GPIO_Port, HOLD_Pin, GPIO_PIN_SET);
}

void IGN_PowerHold_Off(void)
{
    HAL_GPIO_WritePin(HOLD_GPIO_Port, HOLD_Pin, GPIO_PIN_RESET);
}

uint8_t IGN_ShutdownRequested(void)
{
    return s_shutdown_req;
}

void IGN_ClearShutdownRequest(void)
{
    s_shutdown_req = 0;
}

/* Вызывать из HAL_GPIO_EXTI_Callback */
void IGN_OnExti(void)
{
    //if (gpio_pin == IGN_Pin) {
        /* IGN пропал (Falling) */
        s_shutdown_req = 1;
    //}
}


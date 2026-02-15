#include "ds3231.h"
#include "stm32f3xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define DS_ADDR_7B  0x68
#define DS_ADDR     (DS_ADDR_7B << 1)

static inline uint8_t bcd2bin(uint8_t v){ return (uint8_t)(((v >> 4) * 10u) + (v & 0x0Fu)); }
static inline uint8_t bin2bcd(uint8_t v){ return (uint8_t)(((v / 10u) << 4) | (v % 10u)); }

int DS3231_ReadTimeDate(ui_data_t *d)
{
    uint8_t r[7];

    if (HAL_I2C_Mem_Read(&hi2c1, DS_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, r, 7, 100) != HAL_OK)
        return -1;

    /* 0x00..0x02 */
    d->hh = bcd2bin(r[2] & 0x3F);   /* 24h */
    d->mm = bcd2bin(r[1] & 0x7F);
    /* секунды можно игнорить для UI */

    /* 0x04 date, 0x05 month, 0x06 year */
    d->dd   = bcd2bin(r[4] & 0x3F);
    d->mo   = bcd2bin(r[5] & 0x1F);
    d->yyyy = (uint16_t)(2000u + bcd2bin(r[6]));

    return 0;
}

int DS3231_WriteTimeDate(const ui_data_t *d)
{
    /* Пишем регистры 0x00..0x06
       sec, min, hour, dow, date, month, year */
    uint8_t w[7];

    w[0] = bin2bcd(0);                 /* ss = 00 (чтобы красиво) */
    w[1] = bin2bcd(d->mm);
    w[2] = bin2bcd(d->hh);             /* 24h */
    w[3] = bin2bcd(1);                 /* day-of-week = 1 (можно не использовать) */
    w[4] = bin2bcd(d->dd);
    w[5] = bin2bcd(d->mo);             /* century bit = 0 */
    w[6] = bin2bcd((uint8_t)(d->yyyy % 100u));

    if (HAL_I2C_Mem_Write(&hi2c1, DS_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, w, 7, 100) != HAL_OK)
        return -1;

    return 0;
}

int DS3231_SetAlarmEveryMinute(void)
{
    /* Alarm1: 0x07..0x0A
       A1M1..A1M4:
       - секунды должны совпасть (A1M1=0)
       - минуты/часы/дата игнор (A1M2=1,A1M3=1,A1M4=1)
       => каждую минуту на 00 сек
    */
    uint8_t a[4];
    a[0] = bin2bcd(0) & 0x7F;  /* sec=00, A1M1=0 */
    a[1] = 0x80;               /* A1M2=1 */
    a[2] = 0x80;               /* A1M3=1 */
    a[3] = 0x80;               /* A1M4=1 */

    if (HAL_I2C_Mem_Write(&hi2c1, DS_ADDR, 0x07, I2C_MEMADD_SIZE_8BIT, a, 4, 100) != HAL_OK)
        return -1;

    /* Control 0x0E: INTCN=1, A1IE=1 */
    uint8_t ctrl = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, DS_ADDR, 0x0E, I2C_MEMADD_SIZE_8BIT, &ctrl, 1, 100) != HAL_OK)
        return -1;

    ctrl |= (1u << 2); /* INTCN */
    ctrl |= (1u << 0); /* A1IE  */

    if (HAL_I2C_Mem_Write(&hi2c1, DS_ADDR, 0x0E, I2C_MEMADD_SIZE_8BIT, &ctrl, 1, 100) != HAL_OK)
        return -1;

    return 0;
}

int DS3231_ClearAlarm1Flag(void)
{
    uint8_t st = 0;
    if (HAL_I2C_Mem_Read(&hi2c1, DS_ADDR, 0x0F, I2C_MEMADD_SIZE_8BIT, &st, 1, 100) != HAL_OK)
        return -1;

    st &= ~(1u << 0); /* A1F=0 */

    if (HAL_I2C_Mem_Write(&hi2c1, DS_ADDR, 0x0F, I2C_MEMADD_SIZE_8BIT, &st, 1, 100) != HAL_OK)
        return -1;

    return 0;
}

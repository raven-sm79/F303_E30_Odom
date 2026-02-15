#pragma once
#include <stdint.h>
#include <ui.h>

typedef struct {
    uint8_t ss, mm, hh;
    uint8_t dd, mo;
    uint16_t yyyy;
} rtc_time_t;

void DS3231_Init_1Hz_Int(void);
//void DS3231_SetAlarmEveryMinute(void);
//void DS3231_ClearAlarm1Flag(void);
//int  DS3231_ReadTime(rtc_time_t *t);

int  DS3231_ReadTimeDate(ui_data_t *d);
int  DS3231_WriteTimeDate(const ui_data_t *d);

/* Alarm1 каждую минуту (00 секунд) */
int  DS3231_SetAlarmEveryMinute(void);
int  DS3231_ClearAlarm1Flag(void);

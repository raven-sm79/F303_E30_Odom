#ifndef INC_FONT5X7_H_
#define INC_FONT5X7_H_

#include <stdint.h>

/* 5x7, символы 0x20..0x7E (пробел..~)
   Формат: 5 байт на символ, каждый байт = колонка, биты 0..6 = пиксели по Y */
extern const uint8_t font5x7[95][5];

#endif

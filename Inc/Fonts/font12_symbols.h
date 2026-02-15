#pragma once
#include <stdint.h>



#define FONT12_SYMBOLS_W 12
#define FONT12_SYMBOLS_H 18
#define FONT12_SYMBOLS_BPR ((FONT12_SYMBOLS_W + 7) / 8)
#define FONT12_SYMBOLS_SZ (FONT12_SYMBOLS_H * FONT12_SYMBOLS_BPR)
#define FONT12_SYMBOLS_COUNT 17

extern const uint8_t font12_symbols[FONT12_SYMBOLS_COUNT][FONT12_SYMBOLS_SZ];
extern const char font12_symbols_map[FONT12_SYMBOLS_COUNT + 1];
extern const int8_t font12_y_offset[FONT12_SYMBOLS_COUNT];

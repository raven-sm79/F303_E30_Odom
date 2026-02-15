#pragma once
#include <stdint.h>

#define FONT27_DIGITS_W 18
#define FONT27_DIGITS_H 31
#define FONT27_DIGITS_BPR ((FONT27_DIGITS_W + 7) / 8)
#define FONT27_DIGITS_SZ (FONT27_DIGITS_H * FONT27_DIGITS_BPR)

extern const uint8_t font27_digits[10][FONT27_DIGITS_SZ];
extern const char font27_digits_map[11];


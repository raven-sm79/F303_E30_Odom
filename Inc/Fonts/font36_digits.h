#pragma once
#include <stdint.h>

#define FONT36_DIGITS_W 24
#define FONT36_DIGITS_H 36
#define FONT36_DIGITS_BPR ((FONT36_DIGITS_W + 7) / 8)
#define FONT36_DIGITS_SZ (FONT36_DIGITS_H * FONT36_DIGITS_BPR)

extern const uint8_t font36_digits[10][FONT36_DIGITS_SZ];
extern const char font36_digits_map[11];


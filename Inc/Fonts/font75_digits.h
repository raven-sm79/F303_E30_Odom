#pragma once
#include <stdint.h>

#define FONT75_DIGITS_W 42
#define FONT75_DIGITS_H 73
#define FONT75_DIGITS_BPR ((FONT75_DIGITS_W + 7) / 8)
#define FONT75_DIGITS_SZ (FONT75_DIGITS_H * FONT75_DIGITS_BPR)

extern const uint8_t font75_digits[10][FONT75_DIGITS_SZ];
extern const char font75_digits_map[11];


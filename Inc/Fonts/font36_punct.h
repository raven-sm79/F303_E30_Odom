#pragma once
#include <stdint.h>

#define FONT36_PUNCT_W 24
#define FONT36_PUNCT_H 36
#define FONT36_PUNCT_BPR ((FONT36_PUNCT_W + 7) / 8)
#define FONT36_PUNCT_SZ (FONT36_PUNCT_H * FONT36_PUNCT_BPR)

extern const uint8_t font36_punct[4][FONT36_PUNCT_SZ];
extern const char font36_punct_map[5];


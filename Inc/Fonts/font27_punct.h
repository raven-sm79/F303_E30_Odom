#pragma once
#include <stdint.h>

#define FONT27_PUNCT_W 18
#define FONT27_PUNCT_H 31
#define FONT27_PUNCT_BPR ((FONT27_PUNCT_W + 7) / 8)
#define FONT27_PUNCT_SZ (FONT27_PUNCT_H * FONT27_PUNCT_BPR)

extern const uint8_t font27_punct[4][FONT27_PUNCT_SZ];
extern const char font27_punct_map[5];


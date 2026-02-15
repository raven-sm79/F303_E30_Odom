#pragma once
#include <stdint.h>

#define FONT75_PUNCT_W 42
#define FONT75_PUNCT_H 73
#define FONT75_PUNCT_BPR ((FONT75_PUNCT_W + 7) / 8)
#define FONT75_PUNCT_SZ (FONT75_PUNCT_H * FONT75_PUNCT_BPR)

extern const uint8_t font75_punct[4][FONT75_PUNCT_SZ];
extern const char font75_punct_map[5];


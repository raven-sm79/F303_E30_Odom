#pragma once
#include <stdint.h>

typedef struct {
    uint16_t ntc_raw;   // Rank1: CH1 PA0
    uint16_t vbat_raw;  // Rank2: CH2 PA1
} adc2_raw_t;

int ADC2_Read(adc2_raw_t *out);   // читает оба ранга за один старт

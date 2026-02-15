#ifndef INC_ST7789_H_
#define INC_ST7789_H_

#include "stm32f3xx_hal.h"
#include <stdint.h>

void ST7789_Init(SPI_HandleTypeDef *spi);
void ST7789_FillColor(uint16_t color);
void ST7789_SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7789_WriteColorBurst(uint16_t color, uint32_t count);
void ST7789_WritePixels(const uint16_t *pixels, uint32_t count);
void ST7789_BeginData(void);
void ST7789_EndData(void);
void ST7789_WriteDataBytes(const uint8_t *data, uint16_t len);



#endif /* INC_ST7789_H_ */

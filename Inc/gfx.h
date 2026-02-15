#ifndef INC_GFX_H_
#define INC_GFX_H_

#include <stdint.h>

void GFX_DrawPixel(int x, int y, uint16_t color);
void GFX_FillRect(int x, int y, int w, int h, uint16_t color);
void GFX_DrawRect(int x, int y, int w, int h, uint16_t color);
void GFX_DrawLine(int x0, int y0, int x1, int y1, uint16_t color);
void GFX_FillScreen(uint16_t color);
void GFX_DrawLineFast(int x0, int y0, int x1, int y1, uint16_t color);
void GFX_DrawHLine(int x, int y, int w, uint16_t color);
void GFX_DrawVLine(int x, int y, int h, uint16_t color);


#endif

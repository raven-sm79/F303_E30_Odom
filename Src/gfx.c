#include "gfx.h"
#include "st7789.h"

/* Держим размеры тут же (позже вынесем в конфиг/rotation) */
#define LCD_W 240
#define LCD_H 320

static inline int in_bounds(int x, int y)
{
    return (x >= 0 && x < LCD_W && y >= 0 && y < LCD_H);
}

void GFX_DrawPixel(int x, int y, uint16_t color)
{
    if (!in_bounds(x, y)) return;
    ST7789_SetAddrWindow((uint16_t)x, (uint16_t)y, (uint16_t)x, (uint16_t)y);
    ST7789_WriteColorBurst(color, 1);
}

void GFX_FillRect(int x, int y, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;

    int x0 = x, y0 = y;
    int x1 = x + w - 1;
    int y1 = y + h - 1;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 >= LCD_W) x1 = LCD_W - 1;
    if (y1 >= LCD_H) y1 = LCD_H - 1;

    if (x0 > x1 || y0 > y1) return;

    ST7789_SetAddrWindow((uint16_t)x0, (uint16_t)y0, (uint16_t)x1, (uint16_t)y1);
    ST7789_WriteColorBurst(color, (uint32_t)(x1 - x0 + 1) * (uint32_t)(y1 - y0 + 1));
}

void GFX_FillScreen(uint16_t color)
{
    GFX_FillRect(0, 0, LCD_W, LCD_H, color);
}

void GFX_DrawRect(int x, int y, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    GFX_FillRect(x, y, w, 1, color);
    GFX_FillRect(x, y + h - 1, w, 1, color);
    GFX_FillRect(x, y, 1, h, color);
    GFX_FillRect(x + w - 1, y, 1, h, color);
}

/* Брезенхэм */
void GFX_DrawLine(int x0, int y0, int x1, int y1, uint16_t color)
{
    int dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    int sy = (y0 < y1) ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;

    while (1) {
        GFX_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}

void GFX_DrawLineFast(int x0, int y0, int x1, int y1, uint16_t color)
{
    /* Вертикальная */
    if (x0 == x1) {
        int y_min = (y0 < y1) ? y0 : y1;
        int y_max = (y0 > y1) ? y0 : y1;
        GFX_FillRect(x0, y_min, 1, (y_max - y_min + 1), color);
        return;
    }

    /* Горизонтальная */
    if (y0 == y1) {
        int x_min = (x0 < x1) ? x0 : x1;
        int x_max = (x0 > x1) ? x0 : x1;
        GFX_FillRect(x_min, y0, (x_max - x_min + 1), 1, color);
        return;
    }

    /* Диагонали/наклонные — обычный Брезенхэм */
    GFX_DrawLine(x0, y0, x1, y1, color);
}

void GFX_DrawHLine(int x, int y, int w, uint16_t color)
{
    if (w <= 0) return;
    GFX_FillRect(x, y, w, 1, color);
}

void GFX_DrawVLine(int x, int y, int h, uint16_t color)
{
    if (h <= 0) return;
    GFX_FillRect(x, y, 1, h, color);
}


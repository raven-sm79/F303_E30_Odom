#include "text.h"
#include "gfx.h"
#include "font5x7.h"

void TEXT_DrawChar5x7(int x, int y, char c, uint16_t fg, uint16_t bg)
{
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t *cols = font5x7[(uint8_t)c - 0x20];

    /* 5 колонок + 1 пустая колонка (межсимвольный интервал) */
    for (int cx = 0; cx < 5; cx++) {
        uint8_t bits = cols[cx];
        for (int cy = 0; cy < 7; cy++) {
            uint16_t col = (bits & (1u << cy)) ? fg : bg;
            GFX_DrawPixel(x + cx, y + cy, col);
        }
    }
    /* интервал */
    for (int cy = 0; cy < 7; cy++) {
        GFX_DrawPixel(x + 5, y + cy, bg);
    }
}

void TEXT_DrawString5x7(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    while (*s) {
        TEXT_DrawChar5x7(x, y, *s, fg, bg);
        x += 6;
        s++;
    }
}


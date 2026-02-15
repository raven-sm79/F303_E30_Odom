#include "gfx_mono.h"
#include "st7789.h"

/* bmp: 1bpp, строки подряд.
   bpr = bytes per row.
   Бит 7..0 в байте: считаем что левый пиксель = бит7 (MSB-first).
   Если у тебя окажется наоборот — скажешь, я дам переключатель. */

void GFX_DrawMono1BPP(int x, int y,
                      int w, int h, int bpr,
                      const uint8_t *bmp,
                      uint16_t fg, uint16_t bg)
{
    if (w <= 0 || h <= 0) return;

    ST7789_SetAddrWindow((uint16_t)x, (uint16_t)y,
                         (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));

    /* Начинаем поток данных в RAMWR-окно */
    ST7789_BeginData();

    /* Буфер одной строки в RGB565 big-endian байтах */
    /* w может быть большим (75), поэтому делаем буфер по кускам */
    uint8_t line[128]; // 64 пикселя максимум за раз (64*2=128)
    int chunk_px = (int)(sizeof(line) / 2);

    for (int row = 0; row < h; row++) {
        const uint8_t *src = bmp + row * bpr;

        int col = 0;
        while (col < w) {
            int n = w - col;
            if (n > chunk_px) n = chunk_px;

            for (int i = 0; i < n; i++) {
                int xpix = col + i;
                uint8_t byte = src[xpix >> 3];
                uint8_t mask = (uint8_t)(0x80u >> (xpix & 7));
                uint16_t c = (byte & mask) ? fg : bg;

                line[i*2 + 0] = (uint8_t)(c >> 8);
                line[i*2 + 1] = (uint8_t)(c & 0xFF);
            }

            ST7789_WriteDataBytes(line, (uint16_t)(n * 2));
            col += n;
        }
    }

    ST7789_EndData();
}

#include "gfx_mono.h"
#include "st7789.h"



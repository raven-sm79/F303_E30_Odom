#include "font_render.h"
#include "gfx_mono.h"

int FONT_FindInMap(const char *map, char ch)
{
    for (int i = 0; map[i] != 0; i++) {
        if (map[i] == ch) return i;
    }
    return -1;
}

void FONT_DrawMapped1BPP(int x, int y,
                         char ch,
                         const char *map,
                         const int8_t *y_offset,
                         int glyph_w, int glyph_h, int glyph_bpr,
                         const uint8_t *glyph0, int glyph_stride,
                         uint16_t fg, uint16_t bg)
{
    int idx = FONT_FindInMap(map, ch);
    if (idx < 0) return;

    int yo = y_offset ? (int)y_offset[idx] : 0;
    const uint8_t *bmp = glyph0 + (idx * glyph_stride);

    GFX_DrawMono1BPP(x, y - yo, glyph_w, glyph_h, glyph_bpr, bmp, fg, bg);
}

#ifndef INC_FONT_RENDER_H_
#define INC_FONT_RENDER_H_

#include <stdint.h>

int  FONT_FindInMap(const char *map, char ch);
void FONT_DrawMapped1BPP(int x, int y,
                         char ch,
                         const char *map,
                         const int8_t *y_offset,
                         int glyph_w, int glyph_h, int glyph_bpr,
                         const uint8_t *glyph0, int glyph_stride,
                         uint16_t fg, uint16_t bg);

#endif

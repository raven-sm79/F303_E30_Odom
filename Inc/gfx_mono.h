#ifndef INC_GFX_MONO_H_
#define INC_GFX_MONO_H_

#include <stdint.h>

void GFX_DrawMono1BPP(int x, int y,
                      int w, int h, int bpr,
                      const uint8_t *bmp,
                      uint16_t fg, uint16_t bg);



#endif

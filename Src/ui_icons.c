#include "ui_icons.h"
#include "gfx_mono.h"

void UI_DrawIcon36(int x, int y, const uint8_t *icon,
                   uint16_t fg, uint16_t bg)
{
    if (!icon) return;
    GFX_DrawMono1BPP(x, y, UI_ICON_W, UI_ICON_H, UI_ICON_BPR, icon, fg, bg);
}

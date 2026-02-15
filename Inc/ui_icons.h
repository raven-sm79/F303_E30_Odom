#ifndef INC_UI_ICONS_H_
#define INC_UI_ICONS_H_

#include <stdint.h>

/* Все иконки: 36x36, 1bpp */
#define UI_ICON_W   36
#define UI_ICON_H   36
#define UI_ICON_BPR 5
#define UI_ICON_SZ  (UI_ICON_BPR * UI_ICON_H)  /* 180 */

/* Данные */
extern const uint8_t ICON_AB[UI_ICON_SZ];
extern const uint8_t ICON_BENZ[UI_ICON_SZ];
extern const uint8_t ICON_ERROR[UI_ICON_SZ];
extern const uint8_t ICON_GRM[UI_ICON_SZ];
extern const uint8_t ICON_OIL[UI_ICON_SZ];
extern const uint8_t ICON_SERVICE[UI_ICON_SZ];
extern const uint8_t ICON_SPARK[UI_ICON_SZ];
extern const uint8_t ICON_SUT[UI_ICON_SZ];

/* Отрисовка (быстро, через окно и SPI поток) */
void UI_DrawIcon36(int x, int y, const uint8_t *icon,
                   uint16_t fg, uint16_t bg);

#endif

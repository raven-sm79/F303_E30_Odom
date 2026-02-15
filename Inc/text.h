#ifndef INC_TEXT_H_
#define INC_TEXT_H_

#include <stdint.h>

void TEXT_DrawChar5x7(int x, int y, char c, uint16_t fg, uint16_t bg);
void TEXT_DrawString5x7(int x, int y, const char *s, uint16_t fg, uint16_t bg);

#endif

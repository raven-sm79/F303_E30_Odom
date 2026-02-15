#ifndef INC_UI_FONTS_H_
#define INC_UI_FONTS_H_

#include <stdint.h>

/* 12px symbols: "0123456789.-:VC°/" */
void UIF_DrawChar12(int x, int y, char ch, uint16_t fg, uint16_t bg);
void UIF_DrawString12(int x, int y, const char *s, uint16_t fg, uint16_t bg);

/* Digits-only fonts */
void UIF_DrawDigit27(int x, int y, uint8_t d, uint16_t fg, uint16_t bg);
void UIF_DrawDigit36(int x, int y, uint8_t d, uint16_t fg, uint16_t bg);
void UIF_DrawDigit75(int x, int y, uint8_t d, uint16_t fg, uint16_t bg);

/* 75 punct (например ':') */
void UIF_DrawPunct75(int x, int y, char ch, uint16_t fg, uint16_t bg);

/* Helpers for numbers */
void UIF_DrawNumber5_Right27(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg);
void UIF_DrawNumber6_Center36(int center_x, int y, uint32_t val, uint16_t fg, uint16_t bg);

/* 5 digits right-aligned, leading zeros suppressed (spaces) */
void UIF_DrawNumber5_Right27_Trim(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg);
/* Update variant: redraw only changed digits (erase old then draw new) */
void UIF_UpdateNumber5_Right27(int right_x, int y, uint32_t old_val, uint32_t new_val, uint16_t fg, uint16_t bg);

void UIF_DrawNumber6_Right27_Trim(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg);
void UIF_UpdateNumber6_Right27(int right_x, int y, uint32_t old_val, uint32_t new_val, uint16_t fg, uint16_t bg);
void UIF_DrawUInt_Right27(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg);
void UIF_DrawNumber6_Right27_Fixed(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg);

/* Одометр: ровно 6 цифр (с ведущими нулями), по центру */
void UIF_DrawNumber6_Center36_Fixed(int center_x, int y,
                                   uint32_t val,
                                   uint16_t fg, uint16_t bg);

/* Metrics (чтобы main/ui не включал font*.h) */
int UIF_Adv12(void);      // шаг символа 12px (W+1)
int UIF_Adv27(void);      // шаг цифры 27px (W+1)
int UIF_Adv36(void);      // шаг цифры 36px (W+1)
int UIF_Adv75(void);      // шаг цифры 75px (W+2 как в твоём макете)

int UIF_H12(void);
int UIF_H27(void);
int UIF_H36(void);
int UIF_H75(void);
int UIF_H75Punct(void);

#endif

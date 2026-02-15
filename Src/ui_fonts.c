#include "ui_fonts.h"
#include "gfx_mono.h"

/* Подключаем твои данные шрифтов */
#include "Fonts/font12_symbols.h"
#include "Fonts/font27_digits.h"
#include "Fonts/font36_digits.h"
#include "Fonts/font75_digits.h"
#include "Fonts/font75_punct.h"

int UIF_Adv12(void) { return FONT12_SYMBOLS_W + 1; }
int UIF_Adv27(void) { return FONT27_DIGITS_W  + 1; }
int UIF_Adv36(void) { return FONT36_DIGITS_W  + 1; }
int UIF_Adv75(void) { return FONT75_DIGITS_W  + 2; }  // как у тебя dx= W+2

int UIF_H12(void) { return FONT12_SYMBOLS_H; }
int UIF_H27(void) { return FONT27_DIGITS_H; }
int UIF_H36(void) { return FONT36_DIGITS_H; }
int UIF_H75(void) { return FONT75_DIGITS_H; }
int UIF_H75Punct(void) { return FONT75_PUNCT_H; }

/* -------------------- ВНУТРЕННИЕ ХЕЛПЕРЫ -------------------- */

static int find_in_map(const char *map, char ch)
{
    for (int i = 0; map[i] != 0; i++) {
        if (map[i] == ch) return i;
    }
    return -1;
}

/* Универсальная отрисовка глифа по индексу (1bpp) */
static void draw_glyph_1bpp(int x, int y,
                            int w, int h, int bpr,
                            const uint8_t *glyph_base,
                            int glyph_stride,
                            int idx,
                            uint16_t fg, uint16_t bg)
{
    if (idx < 0) return;
    const uint8_t *bmp = glyph_base + (idx * glyph_stride);
    GFX_DrawMono1BPP(x, y, w, h, bpr, bmp, fg, bg);
}

static void num6_to_str_trim(uint32_t v, char out6[6])
{
    if (v > 999999) v %= 1000000;

    out6[5] = (char)('0' + (v % 10)); v /= 10;
    out6[4] = (char)('0' + (v % 10)); v /= 10;
    out6[3] = (char)('0' + (v % 10)); v /= 10;
    out6[2] = (char)('0' + (v % 10)); v /= 10;
    out6[1] = (char)('0' + (v % 10)); v /= 10;
    out6[0] = (char)('0' + (v % 10));

    /* ведущие нули -> пробелы, но не трогаем последний разряд */
    int i = 0;
    while (i < 5 && out6[i] == '0') { out6[i] = ' '; i++; }
}

/* -------------------- FONT12 SYMBOLS -------------------- */

void UIF_DrawChar12(int x, int y, char ch, uint16_t fg, uint16_t bg)
{
    int idx = find_in_map(font12_symbols_map, ch);
    if (idx < 0) return;

    int yo = (int)font12_y_offset[idx];

    draw_glyph_1bpp(
        x,
        y - yo,
        FONT12_SYMBOLS_W,
        FONT12_SYMBOLS_H,
        FONT12_SYMBOLS_BPR,
        &font12_symbols[0][0],
        FONT12_SYMBOLS_SZ,
        idx,
        fg, bg
    );
}

void UIF_DrawString12(int x, int y, const char *s, uint16_t fg, uint16_t bg)
{
    int cx = x;
    while (*s) {
        UIF_DrawChar12(cx, y, *s, fg, bg);
        cx += (FONT12_SYMBOLS_W + 1);
        s++;
    }
}

/* -------------------- DIGITS-ONLY -------------------- */

void UIF_DrawDigit27(int x, int y, uint8_t d, uint16_t fg, uint16_t bg)
{
    if (d > 9) return;
    draw_glyph_1bpp(x, y,
                    FONT27_DIGITS_W, FONT27_DIGITS_H, FONT27_DIGITS_BPR,
                    &font27_digits[0][0], FONT27_DIGITS_SZ,
                    (int)d, fg, bg);
}

void UIF_DrawDigit36(int x, int y, uint8_t d, uint16_t fg, uint16_t bg)
{
    if (d > 9) return;
    draw_glyph_1bpp(x, y,
                    FONT36_DIGITS_W, FONT36_DIGITS_H, FONT36_DIGITS_BPR,
                    &font36_digits[0][0], FONT36_DIGITS_SZ,
                    (int)d, fg, bg);
}

void UIF_DrawDigit75(int x, int y, uint8_t d, uint16_t fg, uint16_t bg)
{
    if (d > 9) return;
    draw_glyph_1bpp(x, y,
                    FONT75_DIGITS_W, FONT75_DIGITS_H, FONT75_DIGITS_BPR,
                    &font75_digits[0][0], FONT75_DIGITS_SZ,
                    (int)d, fg, bg);
}


/* -------------------- FONT75 PUNCT -------------------- */

void UIF_DrawPunct75(int x, int y, char ch, uint16_t fg, uint16_t bg)
{
    /* Если у тебя сейчас только ':' — можно оставить быстрый путь.
       Но сделаем универсально через map. */
    int idx = find_in_map(font75_punct_map, ch);
    if (idx < 0) return;

    draw_glyph_1bpp(x, y,
                    FONT75_PUNCT_W, FONT75_PUNCT_H, FONT75_PUNCT_BPR,
                    &font75_punct[0][0], FONT75_PUNCT_SZ,
                    idx, fg, bg);
}

/* -------------------- NUMBER HELPERS -------------------- */

void UIF_DrawNumber6_Center36(int center_x, int y, uint32_t val, uint16_t fg, uint16_t bg)
{
    if (val > 999999) val %= 1000000;

    char s[7];
    s[6] = 0;
    for (int i = 5; i >= 0; i--) { s[i] = (char)('0' + (val % 10)); val /= 10; }

    int dx = FONT36_DIGITS_W + 1;
    int total_w = 6 * dx - 1;
    int x = center_x - (total_w / 2);

    for (int i = 0; i < 6; i++) {
        UIF_DrawDigit36(x + i*dx, y, (uint8_t)(s[i] - '0'), fg, bg);
    }
}

static void num5_to_str_trim(uint32_t v, char out5[5])
{
    /* 0..99999 -> "  123" (пробелы слева), но 0 -> "    0" */
    if (v > 99999) v %= 100000;

    out5[4] = (char)('0' + (v % 10)); v /= 10;
    out5[3] = (char)('0' + (v % 10)); v /= 10;
    out5[2] = (char)('0' + (v % 10)); v /= 10;
    out5[1] = (char)('0' + (v % 10)); v /= 10;
    out5[0] = (char)('0' + (v % 10));

    /* подавляем ведущие нули -> пробелы */
    int i = 0;
    while (i < 4 && out5[i] == '0') { out5[i] = ' '; i++; }
}

static void draw_digit_or_space27(int x, int y, char ch, uint16_t fg, uint16_t bg)
{
    if (ch == ' ') {
        /* стереть область разряда (фон) */
        draw_glyph_1bpp(x, y,
                        FONT27_DIGITS_W, FONT27_DIGITS_H, FONT27_DIGITS_BPR,
                        &font27_digits[0][0], FONT27_DIGITS_SZ,
                        0, bg, bg); /* любой индекс, но fg=bg, bg=bg -> чистая заливка области глифа */
        return;
    }
    UIF_DrawDigit27(x, y, (uint8_t)(ch - '0'), fg, bg);
}

void UIF_DrawNumber5_Right27_Trim(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg)
{
    char s[5];
    num5_to_str_trim(val, s);

    int dx = UIF_Adv27();               /* W+1 */
    int total_w = 5 * dx - 1;
    int x0 = right_x - total_w;

    for (int i = 0; i < 5; i++) {
        draw_digit_or_space27(x0 + i*dx, y, s[i], fg, bg);
    }
}

void UIF_UpdateNumber5_Right27(int right_x, int y, uint32_t old_val, uint32_t new_val, uint16_t fg, uint16_t bg)
{
    char o[5], n[5];
    num5_to_str_trim(old_val, o);
    num5_to_str_trim(new_val, n);

    int dx = UIF_Adv27();
    int total_w = 5 * dx - 1;
    int x0 = right_x - total_w;

    for (int i = 0; i < 5; i++) {
        if (o[i] == n[i]) continue;

        // стереть старый символ (локально, в пределах разряда)
        draw_digit_or_space27(x0 + i*dx, y, ' ', bg, bg);

        // нарисовать новый
        draw_digit_or_space27(x0 + i*dx, y, n[i], fg, bg);
    }
}

void UIF_DrawUInt_Right27(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg)
{
    /* выводим без ведущих нулей, но выравниваем по правому краю */
    char s[6];
    int n = 0;

    if (val == 0) {
        s[n++] = '0';
    } else {
        char tmp[6];
        int t = 0;
        while (val && t < (int)sizeof(tmp)) {
            tmp[t++] = (char)('0' + (val % 10));
            val /= 10;
        }
        /* разворот */
        while (t--) s[n++] = tmp[t];
    }
    s[n] = 0;

    int dx = FONT27_DIGITS_W + 1;
    int total_w = n * dx - 1;
    int x = right_x - total_w;

    for (int i = 0; i < n; i++) {
        UIF_DrawDigit27(x + i*dx, y, (uint8_t)(s[i] - '0'), fg, bg);
    }
}

void UIF_UpdateNumber6_Right27(int right_x, int y,
                               uint32_t old_val, uint32_t new_val,
                               uint16_t fg, uint16_t bg)
{
    char o[6], n[6];
    num6_to_str_trim(old_val, o);
    num6_to_str_trim(new_val, n);

    int dx = UIF_Adv27();            /* W+1 */
    int total_w = 6 * dx - 1;
    int x0 = right_x - total_w;

    for (int i = 0; i < 6; i++) {
        if (o[i] == n[i]) continue;

        /* стереть разряд */
        draw_digit_or_space27(x0 + i*dx, y, ' ', bg, bg);

        /* нарисовать новый */
        draw_digit_or_space27(x0 + i*dx, y, n[i], fg, bg);
    }
}

void UIF_DrawNumber6_Right27_Trim(int right_x, int y,
                                 uint32_t val,
                                 uint16_t fg, uint16_t bg)
{
    char s[6];
    num6_to_str_trim(val, s);

    int dx = UIF_Adv27();
    int total_w = 6 * dx - 1;
    int x0 = right_x - total_w;

    for (int i = 0; i < 6; i++) {
        draw_digit_or_space27(x0 + i*dx, y, s[i], fg, bg);
    }
}

void UIF_DrawNumber6_Right27_Fixed(int right_x, int y, uint32_t val, uint16_t fg, uint16_t bg)
{
    if (val > 999999) val %= 1000000;

    char s[7];
    s[6]=0;
    for (int i=5;i>=0;i--) { s[i]=(char)('0'+(val%10)); val/=10; }

    int dx = FONT27_DIGITS_W + 1;
    int total_w = 6*dx - 1;
    int x0 = right_x - total_w;

    for (int i=0;i<6;i++) {
        UIF_DrawDigit27(x0 + i*dx, y, (uint8_t)(s[i]-'0'), fg, bg);
    }
}

void UIF_DrawNumber6_Center36_Fixed(int center_x, int y,
                                   uint32_t val,
                                   uint16_t fg, uint16_t bg)
{
    /* фиксируем диапазон 000000..999999 */
    if (val > 999999) val %= 1000000;

    char s[7];
    s[6] = 0;
    for (int i = 5; i >= 0; i--) { s[i] = (char)('0' + (val % 10)); val /= 10; }

    int dx = FONT36_DIGITS_W + 1;
    int total_w = 6 * dx - 1;
    int x = center_x - (total_w / 2);

    for (int i = 0; i < 6; i++) {
        UIF_DrawDigit36(x + i*dx, y, (uint8_t)(s[i] - '0'), fg, bg);
    }
}

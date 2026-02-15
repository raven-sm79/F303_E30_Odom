#include "st7789.h"

/* ==== НАСТРОЙКА ПОД ТВОИ ПИНЫ ==== */
#define LCD_RST_GPIO_Port   GPIOB
#define LCD_RST_Pin         GPIO_PIN_0

#define LCD_DC_GPIO_Port    GPIOB
#define LCD_DC_Pin          GPIO_PIN_1

#define LCD_CS_GPIO_Port    GPIOB
#define LCD_CS_Pin          GPIO_PIN_10

/* Если у тебя другой размер/ориентация — потом поменяем */
#define ST7789_WIDTH   240
#define ST7789_HEIGHT  320

static SPI_HandleTypeDef *lcd_spi;

/* ==== МАКРОСЫ УПРАВЛЕНИЯ ЛИНИЯМИ ==== */
static inline void CS_LOW(void)  { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); }
static inline void CS_HIGH(void) { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); }

static inline void DC_LOW(void)  { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET); }
static inline void DC_HIGH(void) { HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET); }

static inline void RST_LOW(void)  { HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET); }
static inline void RST_HIGH(void) { HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET); }

/* ==== НИЗКОУРОВНЕВЫЙ ОБМЕН ==== */

void ST7789_BeginData(void)
{
    DC_HIGH();
    CS_LOW();
}

void ST7789_EndData(void)
{
    CS_HIGH();
}

void ST7789_WriteDataBytes(const uint8_t *data, uint16_t len)
{
    HAL_SPI_Transmit(lcd_spi, (uint8_t*)data, len, HAL_MAX_DELAY);
}

static void lcd_write_cmd(uint8_t c)
{
    DC_LOW();
    CS_LOW();
    HAL_SPI_Transmit(lcd_spi, &c, 1, HAL_MAX_DELAY);
    CS_HIGH();
}

static void lcd_write_data(const uint8_t *d, uint16_t len)
{
    DC_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(lcd_spi, (uint8_t*)d, len, HAL_MAX_DELAY);
    CS_HIGH();
}

static void lcd_reset(void)
{
    CS_HIGH();
    DC_HIGH();

    RST_LOW();
    HAL_Delay(10);
    RST_HIGH();
    HAL_Delay(120);
}

/* ==== ST7789 КОМАНДЫ ==== */
#define ST7789_SWRESET   0x01
#define ST7789_SLPOUT    0x11
#define ST7789_COLMOD    0x3A
#define ST7789_MADCTL    0x36
#define ST7789_CASET     0x2A
#define ST7789_RASET     0x2B
#define ST7789_RAMWR     0x2C
#define ST7789_DISPON    0x29

void ST7789_SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t data[4];

    lcd_write_cmd(ST7789_CASET);
    data[0] = (uint8_t)(x0 >> 8);
    data[1] = (uint8_t)(x0 & 0xFF);
    data[2] = (uint8_t)(x1 >> 8);
    data[3] = (uint8_t)(x1 & 0xFF);
    lcd_write_data(data, 4);

    lcd_write_cmd(ST7789_RASET);
    data[0] = (uint8_t)(y0 >> 8);
    data[1] = (uint8_t)(y0 & 0xFF);
    data[2] = (uint8_t)(y1 >> 8);
    data[3] = (uint8_t)(y1 & 0xFF);
    lcd_write_data(data, 4);

    lcd_write_cmd(ST7789_RAMWR);
}

void ST7789_Init(SPI_HandleTypeDef *spi)
{
    lcd_spi = spi;

    lcd_reset();

    lcd_write_cmd(ST7789_SWRESET);
    HAL_Delay(150);

    lcd_write_cmd(ST7789_SLPOUT);
    HAL_Delay(120);

    /* Цвет: 16-bit (RGB565) */
    lcd_write_cmd(ST7789_COLMOD);
    {
        uint8_t d = 0x55; // 16-bit
        lcd_write_data(&d, 1);
    }
    HAL_Delay(10);

    /* Ориентация (потом подгоним). Это один из рабочих дефолтов. */
    lcd_write_cmd(ST7789_MADCTL);
    {
        uint8_t d = 0x00; // row/col order default
        lcd_write_data(&d, 1);
    }
    HAL_Delay(10);

    lcd_write_cmd(ST7789_DISPON);
    HAL_Delay(120);
}

void ST7789_FillColor(uint16_t color)
{
    ST7789_SetAddrWindow(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1);

    /* Заполняем экран одним цветом (RGB565) */
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    /* Чтобы не слать по 2 байта 76800 раз — шлём буфером */
    uint8_t buf[64];
    for (int i = 0; i < (int)sizeof(buf); i += 2) {
        buf[i] = hi;
        buf[i + 1] = lo;
    }

    DC_HIGH();
    CS_LOW();
    for (uint32_t px = 0; px < (ST7789_WIDTH * ST7789_HEIGHT); px += (sizeof(buf) / 2)) {
        HAL_SPI_Transmit(lcd_spi, buf, sizeof(buf), HAL_MAX_DELAY);
    }
    CS_HIGH();
}

void ST7789_WriteColorBurst(uint16_t color, uint32_t count)
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color & 0xFF);

    uint8_t buf[64];
    for (int i = 0; i < (int)sizeof(buf); i += 2) {
        buf[i] = hi;
        buf[i + 1] = lo;
    }

    DC_HIGH();
    CS_LOW();
    while (count) {
        uint32_t px_in_buf = sizeof(buf) / 2;
        uint32_t chunk = (count > px_in_buf) ? px_in_buf : count;
        HAL_SPI_Transmit(lcd_spi, buf, (uint16_t)(chunk * 2), HAL_MAX_DELAY);
        count -= chunk;
    }
    CS_HIGH();
}
void ST7789_WritePixels(const uint16_t *pixels, uint32_t count)
{
    /* pixels: RGB565, порядок big-endian по байтам */
    DC_HIGH();
    CS_LOW();

    /* Передаём как поток байт: [hi][lo] */
    while (count) {
        uint16_t chunk = (count > 32) ? 32 : (uint16_t)count; // 32px = 64 bytes
        uint8_t buf[64];
        for (uint16_t i = 0; i < chunk; i++) {
            buf[i*2 + 0] = (uint8_t)(pixels[i] >> 8);
            buf[i*2 + 1] = (uint8_t)(pixels[i] & 0xFF);
        }
        HAL_SPI_Transmit(lcd_spi, buf, chunk * 2, HAL_MAX_DELAY);
        pixels += chunk;
        count -= chunk;
    }

    CS_HIGH();
}



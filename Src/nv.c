#include "nv.h"
#include "stm32f3xx_hal.h"
#include <string.h>
#include <stddef.h>   /* offsetof */

/* hi2c1 должен быть инициализирован в CubeMX */
extern I2C_HandleTypeDef hi2c1;

/* 24C32: 4KB, I2C 7-bit addr 0x50 */
#define EEPROM_ADDR_7B   0x50u
#define EEPROM_ADDR      (EEPROM_ADDR_7B << 1)

/* 24C32 uses 16-bit memory address, page size usually 32 bytes */
#define EEPROM_PAGE_SZ   32u

/* Два слота. Каждый <= 128..256 байт — с запасом */
/* 2 слота */
#define NV_SLOT0_BASE    0x0000
#define NV_SLOT1_BASE    0x0100

#define NV_MAGIC         0x4F444F33u /* 'ODO3' */
#define NV_VERSION       02u

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;

    uint32_t seq;
    uint16_t pulse_rem;
    uint16_t data_len;   /* sizeof(ui_data_t) */

    ui_data_t data;

    uint32_t crc32;      /* CRC32 от всего до crc32 (исключая crc32) */
} nv_blob_t;
#pragma pack(pop)

/* -------- CRC32 (обычный, полином 0xEDB88320) -------- */

static uint32_t crc32_calc(const void *data, uint32_t len)
{
    const uint8_t *p = (const uint8_t*)data;
    uint32_t crc = 0xFFFFFFFFu;

    for (uint32_t i = 0; i < len; i++) {
        crc ^= p[i];
        for (int b = 0; b < 8; b++) {
            uint32_t m = (uint32_t)(-(int32_t)(crc & 1u));
            crc = (crc >> 1) ^ (0xEDB88320u & m);
        }
    }
    return ~crc;
}

/* ---------- EEPROM helpers ---------- */
static int eep_wait_ready(uint32_t timeout_ms)
{
    uint32_t t0 = HAL_GetTick();
    while ((HAL_GetTick() - t0) < timeout_ms) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, EEPROM_ADDR, 1, 5) == HAL_OK)
            return 0;
        HAL_Delay(1);
    }
    return -1;
}

static int eep_read(uint16_t mem, uint8_t *buf, uint16_t len)
{
    if (HAL_I2C_Mem_Read(&hi2c1, EEPROM_ADDR, mem,
                         I2C_MEMADD_SIZE_16BIT, buf, len, 200) != HAL_OK)
        return -1;
    return 0;
}

/* запись с разбиением по страницам */
static int eep_write(uint16_t mem_addr, const uint8_t *buf, uint16_t len)
{
    while (len) {
        uint16_t page_off = (uint16_t)(mem_addr % EEPROM_PAGE_SZ);
        uint16_t chunk = (uint16_t)(EEPROM_PAGE_SZ - page_off);
        if (chunk > len) chunk = len;

        if (HAL_I2C_Mem_Write(&hi2c1, EEPROM_ADDR, mem_addr,
                              I2C_MEMADD_SIZE_16BIT, (uint8_t*)buf, chunk, 200) != HAL_OK)
            return -1;

        /* ждем окончания внутренней записи */
        if (eep_wait_ready(20) < 0) return -2;

        mem_addr = (uint16_t)(mem_addr + chunk);
        buf += chunk;
        len = (uint16_t)(len - chunk);
    }
    return 0;
}


static uint8_t blob_is_valid(const nv_blob_t *b)
{
    if (b->magic != NV_MAGIC) return 0;
    if (b->version != NV_VERSION) return 0;
    if (b->data_len != (uint16_t)sizeof(ui_data_t)) return 0;
    if (b->pulse_rem >= 4838u) return 0;

    uint32_t need = crc32_calc(b, (uint32_t)(sizeof(nv_blob_t) - sizeof(uint32_t)));
    return (need == b->crc32);
}

static int blob_read(uint16_t base, nv_blob_t *out)
{
    return eep_read(base, (uint8_t*)out, (uint16_t)sizeof(nv_blob_t));
}

static int blob_write(uint16_t base, const nv_blob_t *in)
{
    return eep_write(base, (const uint8_t*)in, (uint16_t)sizeof(nv_blob_t));
}

static uint32_t g_last_seq = 0;

/* ---------- module state ---------- */
//static uint8_t  s_have = 0;
//static uint32_t s_seq  = 0;
//static uint16_t s_last_base = NV_SLOT0_BASE;

void NV_Init(void)
{
    /* ничего */
}

/* выбираем самый свежий валидный слот */
int NV_Load(ui_data_t *d, uint16_t *pulse_rem)
{
    nv_blob_t a, b;
    int ra = blob_read(NV_SLOT0_BASE, &a);
    int rb = blob_read(NV_SLOT1_BASE, &b);

    uint8_t va = (ra == 0) ? blob_is_valid(&a) : 0;
    uint8_t vb = (rb == 0) ? blob_is_valid(&b) : 0;

    if (!va && !vb) {
        return -1; /* нет валидных */
    }

    const nv_blob_t *best = 0;

    if (va && vb) {
        best = (a.seq >= b.seq) ? &a : &b;
    } else if (va) {
        best = &a;
    } else {
        best = &b;
    }

    memcpy(d, &best->data, sizeof(ui_data_t));
    *pulse_rem = best->pulse_rem;
    g_last_seq = best->seq;

    return 0;
}

int NV_Save(const ui_data_t *d, uint16_t pulse_rem)
{
    nv_blob_t blob;
    memset(&blob, 0, sizeof(blob));

    blob.magic   = NV_MAGIC;
    blob.version = NV_VERSION;
    blob.seq     = g_last_seq + 1;
    blob.pulse_rem = (pulse_rem < 4838u) ? pulse_rem : 0;
    blob.data_len  = (uint16_t)sizeof(ui_data_t);
    blob.data = *d;

    blob.crc32 = crc32_calc(&blob, (uint32_t)(sizeof(nv_blob_t) - sizeof(uint32_t)));

    /* выбираем слот: чередуем по seq (можно и по "хуже/лучше", но так проще) */
    uint16_t base = (blob.seq & 1u) ? NV_SLOT1_BASE : NV_SLOT0_BASE;

    int r = blob_write(base, &blob);
    if (r == 0) g_last_seq = blob.seq;

    return r;
}

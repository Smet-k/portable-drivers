#ifndef TFT_ST7735_PORT_H
#define TFT_ST7735_PORT_H

#include <stdint.h>
#include <stdbool.h>
#include "tft_st7735_status.h"

typedef tft_st7735_status_t (*tft_st7735_spi_write_fn)(
    const uint8_t *data, uint32_t len, void *ctx
);

typedef tft_st7735_status_t (*tft_st7735_spi_write_read_fn)(
    const uint8_t *tx_data, uint32_t tx_len,
    uint8_t *rx_data, uint32_t rx_len,
    uint32_t timeout_ms, void *ctx
);

typedef void (*tft_st7735_set_dc_fn)(bool is_data, void *ctx);
typedef void (*tft_st7735_set_reset_fn)(bool active, void *ctx);
typedef void (*tft_st7735_set_cs_fn)(bool active, void *ctx); // only if not hardware-managed

typedef uint32_t (*tft_st7735_get_tick_ms_fn)(void);
typedef void (*tft_st7735_delay_ms_fn)(uint32_t ms);

typedef struct {
    tft_st7735_spi_write_fn      spi_write;
    tft_st7735_spi_write_read_fn spi_write_read;
    tft_st7735_set_dc_fn         set_dc;
    tft_st7735_set_reset_fn      set_reset;
    tft_st7735_set_cs_fn         set_cs;
    tft_st7735_delay_ms_fn       delay_ms;
    tft_st7735_get_tick_ms_fn    get_tick_ms;
    void *ctx;
} tft_st7735_io_t;
#endif
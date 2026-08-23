#ifndef BMP280_PORT_H
#define BMP280_PORT_H

#include <stdint.h>
#include "bmp280_status.h" 

typedef bmp280_status_t (*bmp280_i2c_write_read_fn)(
    uint8_t addr,
    const uint8_t *tx_data, uint32_t tx_len,
    uint8_t *rx_data, uint32_t rx_len,
    uint32_t timeout_ms, 
    void* ctx
);

typedef bmp280_status_t (*bmp280_i2c_write_fn)(
    uint8_t addr,
    const uint8_t *data, 
    uint32_t len,
    void *ctx
);

typedef uint32_t (*bmp280_get_tick_ms_fn)(void);

typedef void (*bmp280_delay_ms_fn)(uint32_t ms);

typedef struct {
    bmp280_i2c_write_read_fn i2c_write_read;
    bmp280_i2c_write_fn i2c_write;
    bmp280_delay_ms_fn  delay_ms;
    bmp280_get_tick_ms_fn  get_tick_ms;
    void *ctx;
} bmp280_io_t;

#endif
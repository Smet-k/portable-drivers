#ifndef BMP280_PORT_MOCK_H
#define BMP280_PORT_MOCK_H

#include "bmp280/bmp280_port.h"

/**
 * @brief Wires mock function pointers into io, backed by internal static state.
 */
void bmp280_port_mock_init(bmp280_io_t *io);

/**
 * @brief Queue raw bytes to be returned by the next i2c_write_read call.
 *        Multiple calls queue multiple responses, consumed in FIFO order.
 */
void bmp280_mock_queue_read(const uint8_t *data, uint32_t len);

/**
 * @brief Force the next `count` I2C operations (write or write_read) to fail.
 */
void bmp280_mock_fail_next(uint32_t count);

/**
 * @brief Inspect the most recent i2c_write call's first two bytes
 *        (register, value) — matches this driver's write shape.
 */
uint8_t bmp280_mock_last_write_reg(void);
uint8_t bmp280_mock_last_write_value(void);

/**
 * @brief Advance the mock's internal millisecond clock (used by get_tick_ms).
 */
void bmp280_mock_advance_ms(uint32_t ms);

/**
 * @brief Reset all mock state (queued reads, fail count, tick, last write).
 */
void bmp280_mock_reset(void);

#endif
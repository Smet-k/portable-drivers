#include "bmp280_port_stm32.h"

static bmp280_status_t stm32_i2c_write(uint8_t addr, const uint8_t *data, uint32_t len, void *ctx) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(
        hi2c, addr << 1, (uint8_t *)data, len, HAL_MAX_DELAY);
    return (err == HAL_OK) ? BMP280_OK : BMP280_ERROR;
}

static bmp280_status_t stm32_i2c_write_read(uint8_t addr,
    const uint8_t *tx, uint32_t tx_len,
    uint8_t *rx, uint32_t rx_len,
    uint32_t timeout_ms, void *ctx)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;

    if (tx_len != 1) return BMP280_INVALID_ARG;

    HAL_StatusTypeDef err = HAL_I2C_Mem_Read(
        hi2c, addr << 1, tx[0], I2C_MEMADD_SIZE_8BIT,
        rx, rx_len, timeout_ms);

    return (err == HAL_OK) ? BMP280_OK : BMP280_ERROR;
}

static void stm32_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

static uint32_t stm32_get_tick_ms(void) {
    return HAL_GetTick();
}

void bmp280_port_stm32_init(bmp280_io_t *io, I2C_HandleTypeDef *hi2c) {
    io->i2c_write      = stm32_i2c_write;
    io->i2c_write_read = stm32_i2c_write_read;
    io->delay_ms       = stm32_delay_ms;
    io->get_tick_ms    = stm32_get_tick_ms;
    io->ctx            = hi2c;
}
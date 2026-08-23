#include "lcd_i2c_port_stm32.h"
#include "stm32f4xx_hal.h"

#define I2C_TIMEOUT_MS 50

static lcd_status_t stm32_i2c_write(uint8_t addr, const uint8_t *data, uint16_t len, void *ctx) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(
        hi2c, addr << 1, (uint8_t *)data, len, I2C_TIMEOUT_MS);
    return (err == HAL_OK) ? LCD_OK : LCD_ERROR;
}

static void stm32_delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks) { }
}

static void stm32_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

void lcd_port_stm32_init(lcd_io_t *io, I2C_HandleTypeDef *hi2c) {
    io->i2c_write = stm32_i2c_write;
    io->delay_us  = stm32_delay_us;
    io->delay_ms  = stm32_delay_ms;
    io->ctx       = hi2c;
}
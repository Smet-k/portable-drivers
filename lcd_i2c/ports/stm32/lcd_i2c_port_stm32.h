#ifndef LCD_I2C_PORT_STM32_H
#define LCD_I2C_PORT_STM32_H
#include "stm32f4xx_hal_i2c.h"

static lcd_status_t stm32_i2c_write(uint8_t addr, const uint8_t *data, uint16_t len, void *ctx);
static void stm32_delay_us(uint32_t us);
static void stm32_delay_ms(uint32_t ms);
void lcd_port_stm32_init(lcd_io_t *io, I2C_HandleTypeDef *hi2c);

#endif
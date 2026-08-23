#ifndef LCD_I2C_PORT_STM32_H
#define LCD_I2C_PORT_STM32_H

#include "stm32f4xx_hal.h"
#include "lcd_i2c/lcd_i2c_status.h"
#include "lcd_i2c/lcd_i2c_port.h"

void lcd_port_stm32_init(lcd_io_t *io, I2C_HandleTypeDef *hi2c);

#endif
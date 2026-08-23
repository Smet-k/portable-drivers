#ifndef BMP280_PORT_STM32_H
#define BMP280_PORT_STM32_H

#include "bmp280/bmp280_port.h"
#include "stm32f4xx_hal.h"

/**
 * @brief Wires an STM32 HAL I2C handle into a portable bmp280_io_t.
 *
 * @param[out] io   io struct to populate (usually cfg.io before bmp280_initialize).
 * @param[in]  hi2c HAL I2C handle already initialized by CubeMX (MX_I2Cx_Init()).
 */
void bmp280_port_stm32_init(bmp280_io_t *io, I2C_HandleTypeDef *hi2c);

#endif
#ifndef TFT_ST7735_PORT_STM32_H
#define TFT_ST7735_PORT_STM32_H

#include "tft_st7735/tft_st7735_port.h"
#include "stm32f4xx_hal.h"

/*
 *  @brief Wires an STM32 HAL SPI handle into a portable tft_st7735_io_t.
 *
 *  @param[out] io   io struct to populate (usually cfg.io before tft_st7735_initialize).
 *  @param[in]  hspi HAL SPI handle already initialized by CubeMX (MX 
 */
void tft_st7735_port_stm32_init(tft_st7735_io_t *io, SPI_HandleTypeDef *hspi);



#endif
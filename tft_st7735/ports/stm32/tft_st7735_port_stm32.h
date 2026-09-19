#ifndef TFT_ST7735_PORT_STM32_H
#define TFT_ST7735_PORT_STM32_H

#include "tft_st7735/tft_st7735_port.h"
#include "stm32f4xx_hal.h"


typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} tft_st7735_gpio_line_t;

typedef struct {
    SPI_HandleTypeDef      *hspi;
    tft_st7735_gpio_line_t  dc;
    tft_st7735_gpio_line_t  reset;
    tft_st7735_gpio_line_t  cs;
} tft_st7735_port_stm32_ctx_t;

/*
 *  @brief Wires an STM32 HAL SPI handle + GPIO lines into a portable tft_st7735_io_t.
 *
 *  @param[out] io   io struct to populate (usually cfg.io before tft_st7735_initialize).
 *  @param[in]  ctx  bundle of SPI handle + DC/RESET/CS GPIO lines.
 */
void tft_st7735_port_stm32_init(tft_st7735_io_t *io, tft_st7735_port_stm32_ctx_t *ctx);

#endif
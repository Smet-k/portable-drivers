#include "tft_st7735_port_stm32.h"

static tft_st7735_status_t stm32_spi_write(const uint8_t *data, uint32_t len, void *ctx) {
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)ctx;
    HAL_StatusTypeDef err = HAL_SPI_Transmit(hspi, (uint8_t *)data, len, HAL_MAX_DELAY);
    return (err == HAL_OK) ? TFT_ST7735_OK : TFT_ST7735_ERROR;
}

static tft_st7735_status_t stm32_spi_write_read(
    const uint8_t *tx_data, uint32_t tx_len,
    uint8_t *rx_data, uint32_t rx_len,
    uint32_t timeout_ms, void *ctx)
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)ctx;

    if (tx_len > 0) {
        HAL_StatusTypeDef err = HAL_SPI_Transmit(hspi, (uint8_t *)tx_data, tx_len, timeout_ms);
        if (err != HAL_OK) return TFT_ST7735_ERROR;
    }

    if (rx_len > 0) {
        HAL_StatusTypeDef err = HAL_SPI_Receive(hspi, rx_data, rx_len, timeout_ms);
        if (err != HAL_OK) return TFT_ST7735_ERROR;
    }

    return TFT_ST7735_OK;
}

static void stm32_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}

static uint32_t stm32_get_tick_ms(void) {
    return HAL_GetTick();
}

static void stm32_set_dc(bool is_data, void *ctx) {
    GPIO_TypeDef *dc_port = (GPIO_TypeDef *)ctx;
    uint16_t dc_pin = (uint16_t)(uintptr_t)ctx; // Assuming ctx holds the pin number
    HAL_GPIO_WritePin(dc_port, dc_pin, is_data ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void stm32_set_reset(bool active, void *ctx) {
    GPIO_TypeDef *reset_port = (GPIO_TypeDef *)ctx;
    uint16_t reset_pin = (uint16_t)(uintptr_t)ctx; // Assuming ctx holds the pin number
    HAL_GPIO_WritePin(reset_port, reset_pin, active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void stm32_set_cs(bool active, void *ctx) {
    GPIO_TypeDef *cs_port = (GPIO_TypeDef *)ctx;
    uint16_t cs_pin = (uint16_t)(uintptr_t)ctx; // Assuming ctx holds the pin number
    HAL_GPIO_WritePin(cs_port, cs_pin, active ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void tft_st7735_port_stm32_init(tft_st7735_io_t *io, SPI_HandleTypeDef *hspi) {
    io->spi_write      = stm32_spi_write;
    io->spi_write_read = stm32_spi_write_read;
    io->set_dc         = stm32_set_dc;
    io->set_reset      = stm32_set_reset;
    io->delay_ms       = stm32_delay_ms;
    io->get_tick_ms    = stm32_get_tick_ms;
    io->ctx            = hspi;
}
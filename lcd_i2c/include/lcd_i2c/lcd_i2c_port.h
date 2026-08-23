#ifndef LCD_I2C_PORT_H
#define LCD_I2C_PORT_H

#include <stdint.h>
#include "lcd_i2c_status.h" 

/**
 * @brief I2C write function signature the driver depends on.
 *
 * @param addr 7-bit device address (unshifted).
 * @param data Buffer to transmit.
 * @param len  Number of bytes.
 * @param ctx  Opaque context pointer for the implementer (e.g. HAL handle).
 */
typedef lcd_status_t (*lcd_i2c_write_fn)(uint8_t addr, const uint8_t *data, uint16_t len, void *ctx);

/**
 * @brief Blocking delay in microseconds.
 */
typedef void (*lcd_delay_us_fn)(uint32_t us);

/**
 * @brief Blocking delay in milliseconds.
 */
typedef void (*lcd_delay_ms_fn)(uint32_t ms);

typedef struct {
    lcd_i2c_write_fn i2c_write;
    lcd_delay_us_fn  delay_us;
    lcd_delay_ms_fn  delay_ms;
    void            *ctx;        
} lcd_io_t;


#endif
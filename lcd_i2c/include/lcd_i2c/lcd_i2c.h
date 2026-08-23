#ifndef LCD_I2C_H
#define LCD_I2C_H
/**
 * @defgroup lcd LCD1602 + PCF8574 driver.
 * @brief Driver for LCD display powered by HD44780U, connected through PCF8574.
 * @{
 */
#include "lcd_i2c_status.h"
#include "lcd_i2c_port.h"
#include "stdbool.h"


 /**
  * @brief LCD word wrap options.
  */
typedef enum {
    LCD_WRAP_NONE = 0x00,
    LCD_WRAP_LINE
} lcd_wrap_mode_t;

/**
 * @brief Lines user is able to write to.
 */
typedef enum {
    LCD_LINE_1 = 0x00,
    LCD_LINE_2
} lcd_line_t;

/**
 * @brief LCD font size.
 */
typedef enum {
    LCD_5x8_FONT = 0x00,
    LCD_5x10_FONT
} lcd_font_t;

/**
 * LCD blink settings.
 */
typedef enum {
    LCD_BLINK_DISABLE = 0x00,
    LCD_BLINK_ENABLE
} lcd_blink_t;

/**
 * LCD cursor settings.
 */
typedef enum {
    LCD_CURSOR_DISABLE = 0x00,
    LCD_CURSOR_ENABLE
} lcd_cursor_t;

typedef struct {
    lcd_io_t io;     
    uint8_t i2c_addr;            
    
    uint8_t rows;
    uint8_t cols;

    lcd_line_t line_cfg;
    lcd_font_t font_cfg;
    lcd_wrap_mode_t word_wrap;
} lcd_config_t;

typedef struct {
    lcd_config_t config;
    bool initialized;

    uint8_t cursor_col;
    uint8_t cursor_row;

    lcd_blink_t blink_cfg;
    lcd_cursor_t cursor_cfg;
    bool display_on;
} lcd_handle_t;

/** 
 * @brief Clears display. 
 * 
 * @param[in] lcd LCD handle.
 * 
 */
lcd_status_t lcd_clear(lcd_handle_t* lcd);

/**
 * @brief Prints the provided text on screen.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] text Text to be printed.
 * 
 * 
 * @note Print function supports newline special symbol.
 */
lcd_status_t lcd_print(lcd_handle_t* lcd, const char* text);

/**
 * @brief Prints formatted text on screen.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] fmt Format string.
 * @param[in] ... Additional arguments required by format string.
 * 
 * @return
 *      - ESP_OK: Text printed successfully.
 *      - ESP_ERR_INVALID_ARG: lcd or fmt are NULL.
 *      - ESP_ERR_INVALID_STATE: lcd is uninitialized.
 * 

 */
lcd_status_t lcd_printf(lcd_handle_t* lcd, const char* fmt, ...);

/**
 * @brief Sets the cursor to provided line and offset.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] line Line to be offset from.
 * @param[in] offset Offset from line start.
 * 
 * @return

 */
lcd_status_t lcd_set_cursor(lcd_handle_t* lcd, lcd_line_t line, uint8_t offset);

/**
 * @brief Initializes an LCD handle.
 * 
 * Initializes an LCD handle and performs the 4-bit 
 * initialization procedure.
 * 
 * @param[in, out] lcd LCD handle to be initialized.
 * @param[in] cfg LCD configuration.
 * 

 */
lcd_status_t lcd_initialize(lcd_handle_t* lcd, lcd_config_t* cfg);

/**
 * @brief Sets LCD's cursor setting to provided state.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] enable cursor state.
 * 

 */
lcd_status_t lcd_set_cursor_enabled(lcd_handle_t* lcd, bool enable);

/**
 * @brief Sets LCD's blink setting to provided state.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] enable blink state.
 * 

 */
lcd_status_t lcd_set_blink_enabled(lcd_handle_t* lcd, bool enable);

/**
 * @brief Sets the LCD's display setting to provided state.
 * 
 * @param[in] lcd LCD handle.
 * @param[in] enable display state.
 * 

 */
lcd_status_t lcd_set_display_enabled(lcd_handle_t* lcd, bool enable);

void DWT_Init(void);
/** @} */
#endif
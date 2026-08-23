#include <stdarg.h>
#include <stdio.h>
#include "lcd_i2c/lcd_i2c.h"

#define LCD_DEFAULT_ROWS 0x02
#define LCD_DEFAULT_COLS 0x10
#define LCD_MAX_ROWS     0x04
#define LCD_MAX_COLS     0x28

#define LCD_RS          0x01
#define LCD_RW          0x02
#define LCD_EN          0x04
#define LCD_BACKLIGHT   0x08

#define CMD_CLEAR_DISPLAY    0x01
#define CMD_RETURN_HOME      0x02
#define CMD_ENTRY_MODE_SET   0x04
#define CMD_DISPLAY_CTRL     0x08
#define CMD_CD_SHIFT         0x10
#define CMD_FUNCTION_SET     0x20
#define CMD_SET_SGRAM_ADDR   0x40

#define DL_PARAM(bit) ((bit) << 0x04)
#define N_PARAM(bit)  ((bit) << 0x03)
#define F_PARAM(bit)  ((bit) << 0x02)

#define D_PARAM(bit)  ((bit) << 0x02)
#define C_PARAM(bit)  ((bit) << 0x01)
#define B_PARAM(bit)  ((bit) << 0x00)

#define ID_PARAM(bit) ((bit) << 0x01)
#define S_PARAM(bit)  ((bit) << 0x00)

#define SC_PARAM(bit) ((bit) << 0x03)
#define RL_PARAM(bit) ((bit) << 0x02)

typedef enum {
    LCD_RS_INSTRUCTION = 0x00,
    LCD_RS_DATA
} lcd_rs_t;

static lcd_status_t lcd_write_8_bit(lcd_handle_t* lcd, uint8_t data, lcd_rs_t rs);
static lcd_status_t lcd_write_4_bit(lcd_handle_t* lcd, uint8_t data, lcd_rs_t rs);
static lcd_status_t lcd_pulse_enable(lcd_handle_t* lcd, uint8_t data);
static lcd_status_t lcd_init_DL(lcd_handle_t* lcd);
static lcd_status_t lcd_newline(lcd_handle_t* lcd);
static lcd_status_t lcd_update_display_ctrl(lcd_handle_t* lcd);

static uint8_t lcd_line_addr[2] = {
    0x80,
    0xC0
};

lcd_status_t lcd_initialize(lcd_handle_t* lcd, lcd_config_t* cfg){
    if (cfg == NULL) return LCD_INVALID_ARG;
    if (cfg->io.i2c_write == NULL || cfg->io.delay_us == NULL || cfg->io.delay_ms == NULL)
        return LCD_INVALID_ARG;

    lcd->config = *cfg;

    lcd->initialized = false;
    lcd->cursor_row = 1;
    lcd->cursor_col = 1;

    if (lcd->config.cols == 0) lcd->config.cols = LCD_DEFAULT_COLS;
    if (lcd->config.rows == 0) lcd->config.rows = LCD_DEFAULT_ROWS;

    if (lcd->config.cols > LCD_MAX_COLS ||
        lcd->config.rows > LCD_MAX_ROWS)
        return LCD_INVALID_ARG;

    if (lcd->config.rows > 1 && lcd->config.font_cfg == LCD_5x10_FONT)
        return LCD_INVALID_ARG;

    lcd->config.io.delay_ms(100);

    lcd_status_t err = lcd_init_DL(lcd);
    if (err != LCD_OK) return err;

    uint8_t init_cmd;

    init_cmd = CMD_FUNCTION_SET | DL_PARAM(0) |
                N_PARAM(lcd->config.line_cfg) |
                F_PARAM(lcd->config.font_cfg);
    err = lcd_write_8_bit(lcd, init_cmd, LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(50);

    init_cmd = CMD_DISPLAY_CTRL | D_PARAM(1) | C_PARAM(0) | B_PARAM(0);
    err = lcd_write_8_bit(lcd, init_cmd, LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(50);

    init_cmd = CMD_CLEAR_DISPLAY;
    err = lcd_write_8_bit(lcd, init_cmd, LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_ms(10);

    init_cmd = CMD_ENTRY_MODE_SET | ID_PARAM(1) | S_PARAM(0);
    err = lcd_write_8_bit(lcd, init_cmd, LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(50);

    lcd->initialized = true;

    return LCD_OK;
}

lcd_status_t lcd_print(lcd_handle_t* lcd, const char* text){
    if(lcd == NULL || text == NULL) return LCD_INVALID_ARG;
    if(!lcd->initialized) return LCD_NOT_INITIALIZED;
    lcd_status_t err;
    while(*text){
        if(*text == '\n'){
            err = lcd_newline(lcd);
            if(err != LCD_OK) return err;
            text++;
            continue;
        }

        err = lcd_write_8_bit(lcd, *text++, LCD_RS_DATA);
        if(err != LCD_OK) return err;

        lcd->cursor_col++;

        if(lcd->config.word_wrap == LCD_WRAP_LINE && lcd->cursor_col > lcd->config.cols) {
            err = lcd_newline(lcd);
            if(err != LCD_OK) return err;
        }
    }
    return LCD_OK;
}

lcd_status_t lcd_printf(lcd_handle_t* lcd, const char* fmt, ...){
    if(lcd == NULL || fmt == NULL) return LCD_INVALID_ARG;
    if(!lcd->initialized) return LCD_NOT_INITIALIZED;

    char buffer[LCD_MAX_ROWS * LCD_MAX_COLS];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len < 0) return LCD_ERROR;
    if ((size_t)len >= sizeof(buffer)) return LCD_ERROR;

    return lcd_print(lcd, buffer);
}

lcd_status_t lcd_set_cursor(lcd_handle_t* lcd, lcd_line_t line, uint8_t offset){
    if(lcd == NULL) return LCD_INVALID_ARG;
    if(!lcd->initialized) return LCD_NOT_INITIALIZED;

    if(line != LCD_LINE_1 && line != LCD_LINE_2)
        return LCD_INVALID_ARG;

    if(offset >= 16)
        return LCD_INVALID_ARG;

    lcd->cursor_row = line + 1;
    lcd->cursor_col = offset;

    uint8_t address = lcd_line_addr[line] + offset;
    lcd_status_t err = lcd_write_8_bit(lcd, address, LCD_RS_INSTRUCTION);
    lcd->config.io.delay_us(50);
    return err;
}

lcd_status_t lcd_clear(lcd_handle_t* lcd){
    if(lcd == NULL) return LCD_INVALID_ARG;
    if(!lcd->initialized) return LCD_NOT_INITIALIZED;

    lcd_status_t err = lcd_write_8_bit(lcd, CMD_CLEAR_DISPLAY, LCD_RS_INSTRUCTION);
    lcd->config.io.delay_ms(10);
    if(err != LCD_OK) return err;

    lcd->cursor_col = 1;
    lcd->cursor_row = 1;

    return LCD_OK;
}

static lcd_status_t lcd_newline(lcd_handle_t* lcd) {
    lcd->cursor_col = 1;
    lcd->cursor_row++;

    if(lcd->cursor_row > lcd->config.rows) return LCD_INVALID_ARG;

    lcd_status_t err = lcd_write_8_bit(lcd, lcd_line_addr[lcd->cursor_row - 1], LCD_RS_INSTRUCTION);
    lcd->config.io.delay_us(50);
    return err;
}

static lcd_status_t lcd_init_DL(lcd_handle_t* lcd){
    lcd_status_t err;

    err = lcd_write_4_bit(lcd, CMD_FUNCTION_SET | DL_PARAM(1), LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(5000);

    err = lcd_write_4_bit(lcd, CMD_FUNCTION_SET | DL_PARAM(1), LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(150);

    err = lcd_write_4_bit(lcd, CMD_FUNCTION_SET | DL_PARAM(1), LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(150);

    err = lcd_write_4_bit(lcd, CMD_FUNCTION_SET | DL_PARAM(0), LCD_RS_INSTRUCTION);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(150);

    return LCD_OK;
}

static lcd_status_t lcd_pulse_enable(lcd_handle_t* lcd, uint8_t data){
    uint8_t cmd = data | LCD_EN;
    lcd_status_t err = lcd->config.io.i2c_write(
        lcd->config.i2c_addr, &cmd, 1, lcd->config.io.ctx);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(1);

    cmd = data & ~LCD_EN;
    err = lcd->config.io.i2c_write(
        lcd->config.i2c_addr, &cmd, 1, lcd->config.io.ctx);
    if(err != LCD_OK) return err;
    lcd->config.io.delay_us(50);

    return LCD_OK;
}

static lcd_status_t lcd_write_8_bit(lcd_handle_t* lcd, uint8_t data, lcd_rs_t rs){
    lcd_status_t err = lcd_write_4_bit(lcd, data & 0xF0, rs);
    if(err != LCD_OK) return err;

    return lcd_write_4_bit(lcd, (data << 4) & 0xF0, rs);
}

static lcd_status_t lcd_write_4_bit(lcd_handle_t* lcd, uint8_t data, lcd_rs_t rs){
    uint8_t msg = 0;

    msg |= (data & 0xF0);
    if (rs) msg |= LCD_RS;
    msg |= LCD_BACKLIGHT;
    msg &= ~LCD_RW;

    lcd_status_t err = lcd->config.io.i2c_write(
        lcd->config.i2c_addr, &msg, 1, lcd->config.io.ctx);
    if (err != LCD_OK) return err;

    return lcd_pulse_enable(lcd, msg);
}

lcd_status_t lcd_set_cursor_enabled(lcd_handle_t* lcd, bool enable) {
    if (lcd == NULL) return LCD_INVALID_ARG;
    lcd->cursor_cfg = enable;
    return lcd_update_display_ctrl(lcd);
}

lcd_status_t lcd_set_blink_enabled(lcd_handle_t* lcd, bool enable) {
    if (lcd == NULL) return LCD_INVALID_ARG;
    lcd->blink_cfg = enable;
    return lcd_update_display_ctrl(lcd);
}

lcd_status_t lcd_set_display_enabled(lcd_handle_t* lcd, bool enable) {
    if (lcd == NULL) return LCD_INVALID_ARG;
    lcd->display_on = enable;
    return lcd_update_display_ctrl(lcd);
}

static lcd_status_t lcd_update_display_ctrl(lcd_handle_t* lcd){
    uint8_t cmd = CMD_DISPLAY_CTRL;

    if (lcd->display_on) cmd |= D_PARAM(1);
    if (lcd->cursor_cfg) cmd |= C_PARAM(1);
    if (lcd->blink_cfg)  cmd |= B_PARAM(1);

    lcd_status_t err = lcd_write_8_bit(lcd, cmd, LCD_RS_INSTRUCTION);
    lcd->config.io.delay_us(50);
    return err;
}
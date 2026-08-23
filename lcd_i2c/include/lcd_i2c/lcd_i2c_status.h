#ifndef LCD_I2C_STATUS_H
#define LCD_I2C_STATUS_H

typedef enum
{
    LCD_OK = 0,
    LCD_ERROR,
    LCD_TIMEOUT,
    LCD_INVALID_ARG,
    LCD_NOT_INITIALIZED
} lcd_status_t;

#endif
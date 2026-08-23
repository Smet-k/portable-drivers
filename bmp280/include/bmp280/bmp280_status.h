#ifndef BMP280_STATUS_H
#define BMP280_STATUS_H

typedef enum
{
    BMP280_OK = 0,
    BMP280_ERROR,
    BMP280_TIMEOUT,
    BMP280_INVALID_ARG,
    BMP280_NOT_INITIALIZED
} bmp280_status_t;

#endif
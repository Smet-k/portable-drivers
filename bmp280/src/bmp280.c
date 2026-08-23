#include "bmp280/bmp280.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define BMP_FORCE_TIMEOUT 100

#define TEMP_CALIBRATION 0x88
#define PRESS_CALIBRATION 0x8E

#define RESET_REG 0xE0
#define STATUS_REG 0xF3
#define CONTROL_REG 0xF4
#define CONFIG_REG 0xF5
#define PRESS_REG 0xF7

#define T_OSRS_X2 0x02
#define P_OSRS_X16 0x05
#define FILTER_X4 0x02

#define CONFIG_T_SB_POS 0x05
#define CONFIG_FILTER_POS 0x02

#define CTRL_OSRS_T_POS 0x05
#define CTRL_OSRS_P_POS 0x02

#define RESET_VALUE 0xB6

#define STATUS_MEASURING_POS 0x03
#define STATUS_MEASURING (1 << STATUS_MEASURING_POS)

/**
 * @brief Read calibration coefficients from BMP280.
 *
 * @param [in,out] bmp280 Initilized device handle.
 *
 * @return
 *      - BMP280_OK on success
 *      - BMP280_ERROR on invalid calibration data
 *      - BMP280_TIMEOUT on I2C timeout
 *
 * @note Must be called once during initialization.
 */
static bmp280_status_t bmp280_load_calibration(bmp280_t* bmp280);

/**
 * @brief Compensate raw temperature reading.
 *
 * Implements compensation formula from BMP280 datasheet.
 * Updates t_fine inside device handle.
 *
 * @param[in,out] dev BMP280 device handle.
 * @param[in] raw_temp Raw ADC temperature value.
 *
 * @return Temperature in °C units.
 */
static int32_t bmp280_compensate_t(bmp280_t* dev, const int raw_temp);

/**
 * @brief Compensate raw pressure reading.
 *
 * Requires valid t_fine value from temperature compensation.
 *
 * @param[in] dev BMP280 device handle.
 * @param[in] raw_press Raw ADC pressure value.
 *
 * @return Pressure in Pa.
 *
 * @note bmp280_compensate_t() must be called successfully at least once before this function (to set out the t_fine)
 */
static uint32_t bmp280_compensate_p(bmp280_t* dev, const int32_t raw_press);

static bmp280_status_t bmp280_i2c_write(bmp280_t* bmp280, const uint8_t *data, uint32_t len) {
    return bmp280->config.io.i2c_write(
        bmp280->config.i2c_addr, data, len, bmp280->config.io.ctx);
}

static bmp280_status_t bmp280_i2c_write_read(bmp280_t* bmp280,
    const uint8_t *tx, uint32_t tx_len,
    uint8_t *rx, uint32_t rx_len,
    uint32_t timeout_ms)
{
    return bmp280->config.io.i2c_write_read(
        bmp280->config.i2c_addr, tx, tx_len, rx, rx_len, timeout_ms, bmp280->config.io.ctx);
}

bmp280_status_t bmp280_initialize(bmp280_t* bmp280, const bmp280_config_t* cfg) {
    if(!cfg) return BMP280_INVALID_ARG;
    if(!cfg->io.i2c_write || !cfg->io.i2c_write_read || 
       !cfg->io.delay_ms || !cfg->io.get_tick_ms) return BMP280_INVALID_ARG;
    
    bmp280_status_t err;

    bmp280->config = *cfg;
    bmp280->initialized = false;


    err = bmp280_load_calibration(bmp280);
    if(err != BMP280_OK) return err;

    if (bmp280->config.work_mode == BMP_NORMAL) {
        uint8_t tx_buf[2];
        tx_buf[0] = CONFIG_REG;
        tx_buf[1] = (BMP_OSRS_X2 << CONFIG_T_SB_POS) | (BMP_OSRS_X16 << CONFIG_FILTER_POS);

        err = bmp280_i2c_write(bmp280, tx_buf, sizeof(tx_buf));
        if (err != BMP280_OK) return err;

        tx_buf[0] = CONTROL_REG;
        tx_buf[1] = (BMP_OSRS_X1 << CTRL_OSRS_T_POS) | (BMP_OSRS_X1 << CTRL_OSRS_P_POS) | bmp280->config.work_mode;
        err = bmp280_i2c_write(bmp280, tx_buf, sizeof(tx_buf));
        if (err != BMP280_OK) return err;
    }

    bmp280->initialized = true;
    return BMP280_OK;
}

bmp280_status_t bmp280_read(bmp280_t* bmp280, bmp280_measurements* measurements) {
    bmp280_status_t err;
    if(!bmp280) return BMP280_INVALID_ARG;
    if (!bmp280->initialized) {
        printf("Measurement error, bmp280 is uninitialized!\n");
        return BMP280_NOT_INITIALIZED;
    }

    uint8_t reg = PRESS_REG;
    uint8_t data[6];

    err = bmp280_i2c_write_read(bmp280, &reg, 1, data, 6, 100);

    if (err != BMP280_OK) {
        printf("I2C error: %d\n", (int)err);
        return err;
    }

    int32_t raw_pressure =
        ((int32_t)data[0] << 12) |
        ((int32_t)data[1] << 4) |
        ((int32_t)data[2] >> 4);

    int32_t raw_temperature =
        ((int32_t)data[3] << 12) |
        ((int32_t)data[4] << 4) |
        ((int32_t)data[5] >> 4);

    int32_t temperature = bmp280_compensate_t(bmp280, raw_temperature);
    uint32_t pressure = bmp280_compensate_p(bmp280, raw_pressure);

    measurements->temperature = temperature / 100.0;
    measurements->pressure = pressure / 256.0;

    return err;
}

bmp280_status_t bmp280_force_measurement(bmp280_t* bmp280, const uint16_t timeout_ms) {
    if(!bmp280 || bmp280->config.work_mode != BMP_FORCE) return BMP280_INVALID_ARG;
    if (!bmp280->initialized) return BMP280_NOT_INITIALIZED;

    bmp280_status_t err;
    uint8_t tx_buf[2] = {
        CONTROL_REG,
        (1 << 5) | (1 << 2) | bmp280->config.work_mode};

    err = bmp280_i2c_write(bmp280, tx_buf, sizeof(tx_buf));

    if (err != BMP280_OK)
        return err;

    uint8_t status = 0;
    uint32_t start = bmp280->config.io.get_tick_ms();
    do {
        if (bmp280->config.io.get_tick_ms() - start > timeout_ms)
            return BMP280_TIMEOUT;

        tx_buf[0] = STATUS_REG;
        err = bmp280_i2c_write_read(bmp280, tx_buf, 1, &status, 1, 5);
        if (err != BMP280_OK) return err;

        bmp280->config.io.delay_ms(5);
    } while (status & STATUS_MEASURING);
    return BMP280_OK;
}

bmp280_status_t bmp280_reset(bmp280_t* bmp280) {
    if (!bmp280)
        return BMP280_INVALID_ARG;

    uint8_t reset_buf[2] = {
        RESET_REG,
        RESET_VALUE};

    bmp280_status_t err = bmp280_i2c_write(bmp280, reset_buf, sizeof(reset_buf));

    if (err != BMP280_OK)
        return err;

    bmp280->config.io.delay_ms(5);

    bmp280->initialized = false;

    return BMP280_OK;
}

static bmp280_status_t bmp280_load_calibration(bmp280_t* bmp280) {
    bmp280_status_t err;
    uint8_t config_temp = TEMP_CALIBRATION;
    uint8_t buf[6];

    err = bmp280_i2c_write_read(bmp280, &config_temp, 1, buf, 6, 100);
    if (err != BMP280_OK)
        return err;

    bmp280->calibration.dig_T1 = (uint16_t)(buf[1] << 8 | buf[0]);
    bmp280->calibration.dig_T2 = (int16_t)(buf[3] << 8 | buf[2]);
    bmp280->calibration.dig_T3 = (int16_t)(buf[5] << 8 | buf[4]);

    uint8_t config_pres = PRESS_CALIBRATION;
    uint8_t buf_pres[18];

    err = bmp280_i2c_write_read(bmp280, &config_pres, 1, buf_pres, 18, 100);

    if (err != BMP280_OK)
        return err;

    bmp280->calibration.dig_P1 = (uint16_t)(buf_pres[1] << 8 | buf_pres[0]);
    bmp280->calibration.dig_P2 = (int16_t)(buf_pres[3] << 8 | buf_pres[2]);
    bmp280->calibration.dig_P3 = (int16_t)(buf_pres[5] << 8 | buf_pres[4]);
    bmp280->calibration.dig_P4 = (int16_t)(buf_pres[7] << 8 | buf_pres[6]);
    bmp280->calibration.dig_P5 = (int16_t)(buf_pres[9] << 8 | buf_pres[8]);
    bmp280->calibration.dig_P6 = (int16_t)(buf_pres[11] << 8 | buf_pres[10]);
    bmp280->calibration.dig_P7 = (int16_t)(buf_pres[13] << 8 | buf_pres[12]);
    bmp280->calibration.dig_P8 = (int16_t)(buf_pres[15] << 8 | buf_pres[14]);
    bmp280->calibration.dig_P9 = (int16_t)(buf_pres[17] << 8 | buf_pres[16]);

    return err;
}

static int32_t bmp280_compensate_t(bmp280_t* dev, const int raw_temp) {
    int32_t var1, var2, T;
    var1 = ((((raw_temp >> 3) - ((int32_t)dev->calibration.dig_T1 << 1))) *
            ((int32_t)dev->calibration.dig_T2)) >>
           11;
    var2 = (((((raw_temp >> 4) - ((int32_t)dev->calibration.dig_T1)) *
              ((raw_temp >> 4) - ((int32_t)dev->calibration.dig_T1))) >>
             12) *
            ((int32_t)dev->calibration.dig_T3)) >>
           14;

    dev->t_fine = var1 + var2;
    T = (dev->t_fine * 5 + 128) >> 8;
    return T;
}

static uint32_t bmp280_compensate_p(bmp280_t* dev, const int32_t raw_press) {
    if (!dev->t_fine) {
        printf("t_fine was not initialized, temperature measurement should be called first.\n");
        return 0;
    }
    int64_t var1, var2, p;
    var1 = ((int64_t)dev->t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dev->calibration.dig_P6;
    var2 = var2 + ((var1 * (int64_t)dev->calibration.dig_P5) << 17);
    var2 = var2 + (((int64_t)dev->calibration.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dev->calibration.dig_P3) >> 8) +
           ((var1 * (int64_t)dev->calibration.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dev->calibration.dig_P1) >> 33;
    if (var1 == 0) return 0;
    p = 1048576 - raw_press;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dev->calibration.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dev->calibration.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dev->calibration.dig_P7) << 4);
    return (uint32_t)p;
}
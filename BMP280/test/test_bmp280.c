#include "bmp280/bmp280.h"
#include "bmp280_port_mock.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_calibration_decode(void) {
    bmp280_mock_reset();

    bmp280_config_t cfg = {0};
    bmp280_port_mock_init(&cfg.io);
    cfg.i2c_addr = 0x76;
    cfg.work_mode = BMP_SLEEP; // avoid extra CONFIG/CONTROL writes for this test

    // dig_T1..T3: little-endian pairs. Chosen so decode is easy to verify by hand.
    // dig_T1 = 0x0102 (unsigned), dig_T2 = 0x0304 (signed), dig_T3 = 0xFFFE (signed = -2)
    uint8_t temp_calib[6] = {
        0x02, 0x01,   // dig_T1 low, high -> 0x0102
        0x04, 0x03,   // dig_T2 low, high -> 0x0304
        0xFE, 0xFF    // dig_T3 low, high -> 0xFFFE (-2 as int16_t)
    };
    bmp280_mock_queue_read(temp_calib, sizeof(temp_calib));

    // dig_P1..P9: just zero-fill for this test, we only care about T here.
    uint8_t press_calib[18] = {0};
    bmp280_mock_queue_read(press_calib, sizeof(press_calib));

    bmp280_t bmp280;
    bmp280_status_t status = bmp280_initialize(&bmp280, &cfg);

    assert(status == BMP280_OK);
    assert(bmp280.initialized == true);
    assert(bmp280.calibration.dig_T1 == 0x0102);
    assert(bmp280.calibration.dig_T2 == 0x0304);
    assert(bmp280.calibration.dig_T3 == -2);

    printf("test_calibration_decode: PASS\n");
}

static void test_initialize_null_args(void) {
    bmp280_mock_reset();

    bmp280_t bmp280;
    assert(bmp280_initialize(NULL, NULL) == BMP280_INVALID_ARG);
    assert(bmp280_initialize(&bmp280, NULL) == BMP280_INVALID_ARG);

    printf("test_initialize_null_args: PASS\n");
}

static void test_initialize_i2c_failure(void) {
    bmp280_mock_reset();

    bmp280_config_t cfg = {0};
    bmp280_port_mock_init(&cfg.io);
    cfg.i2c_addr = 0x76;
    cfg.work_mode = BMP_SLEEP;

    bmp280_mock_fail_next(1); // fail the first calibration read

    bmp280_t bmp280;
    bmp280_status_t status = bmp280_initialize(&bmp280, &cfg);

    assert(status == BMP280_ERROR);
    assert(bmp280.initialized == false);

    printf("test_initialize_i2c_failure: PASS\n");
}

int main(void) {
    test_calibration_decode();
    test_initialize_null_args();
    test_initialize_i2c_failure();

    printf("All tests passed.\n");
    return 0;
}
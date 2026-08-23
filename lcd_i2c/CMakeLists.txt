cmake_minimum_required(VERSION 3.16)
project(lcd_i2c_driver LANGUAGES C)

add_library(lcd_i2c STATIC
    src/lcd_i2c.c
)

target_include_directories(lcd_i2c PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(lcd_i2c PUBLIC c_std_11)

target_compile_options(lcd_i2c PRIVATE -Wall -Wextra -Wpedantic)

option(LCD_I2C_BUILD_STM32_PORT "Build STM32 port" OFF)

if(LCD_I2C_BUILD_STM32_PORT)
    add_library(lcd_i2c_port_stm32 STATIC
        ports/stm32/lcd_i2c_port_stm32.c
    )
    target_include_directories(lcd_i2c_port_stm32
    PUBLIC ports/stm32
    )
    if(TARGET stm32cubemx)
        target_link_libraries(lcd_i2c_port_stm32 PUBLIC stm32cubemx)
    else()
        message(WARNING "STM32 HAL library not found. Please link it manually.")
    endif()
    target_link_libraries(lcd_i2c_port_stm32 PUBLIC lcd_i2c)
endif()

if(LCD_I2C_BUILD_TESTS)
    add_library(lcd_i2c_port_mock STATIC
        ports/mock/lcd_i2c_port_mock.c
    )
    target_include_directories(lcd_i2c_port_mock PUBLIC ports/mock)
    target_link_libraries(lcd_i2c_port_mock PUBLIC lcd_i2c)

    enable_testing()
    add_executable(test_lcd_i2c test/test_lcd_i2c.c)
    target_link_libraries(test_lcd_i2c PRIVATE i2c_lcd lcd_i2c_port_mock)
    add_test(NAME lcd_i2c_tests COMMAND test_lcd_i2c)
endif()
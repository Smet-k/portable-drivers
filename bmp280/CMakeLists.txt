cmake_minimum_required(VERSION 3.16)
project(bmp280_driver LANGUAGES C)

add_library(bmp280 STATIC
    src/bmp280.c
)

target_include_directories(bmp280 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(bmp280 PUBLIC c_std_11) # Whats this?

target_compile_options(bmp280 PRIVATE -Wall -Wextra -Wpedantic)

option(BMP280_BUILD_STM32_PORT "Build STM32 port" OFF)

if(BMP280_BUILD_STM32_PORT)
    add_library(bmp280_port_stm32 STATIC
        ports/stm32/bmp280_port_stm32.c
    )
    target_include_directories(bmp280_port_stm32
    PUBLIC ports/stm32
    )
    if(TARGET stm32cubemx)
        target_link_libraries(bmp280_port_stm32 PUBLIC stm32cubemx)
    else()
        message(WARNING "STM32 HAL library not found. Please link it manually.")
    endif()
    target_link_libraries(bmp280_port_stm32 PUBLIC bmp280)
endif()

if(BMP280_BUILD_TESTS)
    add_library(bmp280_port_mock STATIC
        ports/mock/bmp280_port_mock.c
    )
    target_include_directories(bmp280_port_mock PUBLIC ports/mock)
    target_link_libraries(bmp280_port_mock PUBLIC bmp280)

    enable_testing()
    add_executable(test_bmp280 test/test_bmp280.c)
    target_link_libraries(test_bmp280 PRIVATE bmp280 bmp280_port_mock)
    add_test(NAME bmp280_tests COMMAND test_bmp280)
endif()
cmake_minimum_required(VERSION 3.16)
project(tft_st7735_driver LANGUAGES C)

add_library(tft_st7735 STATIC
    src/tft_st7735.c
)

target_include_directories(tft_st7735 PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(tft_st7735 PUBLIC c_std_11)

target_compile_options(tft_st7735 PRIVATE -Wall -Wextra -Wpedantic)

option(TFT_ST7735_BUILD_STM32_PORT "Build STM32 port" OFF)

if(TFT_ST7735_BUILD_STM32_PORT)
    add_library(tft_st7735_port_stm32 STATIC
        ports/stm32/tft_st7735_port_stm32.c
    )
    target_include_directories(tft_st7735_port_stm32 
    PUBLIC ports/stm32
    )
    if(TARGET stm32cubemx)
        target_link_libraries(tft_st7735_port_stm32 PUBLIC stm32cubemx)
    else()
        message(WARNING "STM32 HAL library not found. Please link it manually.")
    endif()
    target_link_libraries(tft_st7735_port_stm32 PUBLIC tft_st7735)
endif()

if(TFT_ST7735_TESTS)
    add_library(tft_st7735_port_mock STATIC
        ports/mock/tft_st7735_port_mock.c
    )
    target_include_directories(tft_st7735_port_mock PUBLIC ports/mock)
    target_link_libraries(tft_st7735_port_mock PUBLIC tft_st7735)

    enable_testing()
    add_executable(test_tft_st7735 test/test_tft_st7735.c)
    target_link_libraries(test_tft_st7735 PRIVATE tft_st7735 tft_st7735_port_mock)
    add_test(NAME tft_st7735_tests COMMAND test_tft_st7735)
endif()
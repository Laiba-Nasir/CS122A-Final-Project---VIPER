cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(viper C CXX ASM)
pico_sdk_init()

add_executable(viper
    src/main/main.c
    src/camera_config/ov7670_init.c
    src/spi/spi_master.c
    src/servo/servo.c
    src/laser/laser.c
    src/buzzer/buzzer.c
)

target_include_directories(viper PRIVATE
    include
    src/camera_config
    src/spi
    src/servo
    src/laser
    src/buzzer
)

target_link_libraries(viper
    pico_stdlib
    hardware_i2c
    hardware_spi
    hardware_pwm
    hardware_clocks
)

pico_enable_stdio_usb(viper 1)
pico_enable_stdio_uart(viper 0)
pico_add_extra_outputs(viper)
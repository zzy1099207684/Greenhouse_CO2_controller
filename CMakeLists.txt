# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

SET(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/FreeRTOS-KernelV10.6.2)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(Greenhouse_CO2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/ui/ui.cpp
        rp2040-freertos/src/i2c/PicoI2C.cpp
        rp2040-freertos/src/display/ssd1306os.cpp
        rp2040-freertos/src/display/mono_vlsb.cpp
        rp2040-freertos/src/display/framebuf.cpp
        rp2040-freertos/src/Fmutex.cpp
)



include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/modbus
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/i2c
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/display
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/uart
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/ipstack
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/modbus
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/i2c
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/display
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/uart
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/ipstack

)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        hardware_pwm
        hardware_gpio
        hardware_uart
        hardware_i2c
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
)

# Disable usb output, enable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
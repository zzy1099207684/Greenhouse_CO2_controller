# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Set board type because we are building for PicoW
set(PICO_BOARD pico_w)

SET(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/FreeRTOS-KernelV10.6.2)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include(${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C standards
project(Greenhouse_CO2 C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format
        -Wno-unused-function
        -Wno-maybe-uninitialized
)

# Create a library with all your application code
# This library can be used by both the main executable and tests
add_library(${PROJECT_NAME}_lib
        src/Greenhouse_Monitor/GreenhouseMonitor.cpp
        src/Greenhouse_Monitor/GreenhouseMonitor.h
        src/EEPROM/EEPROM.cpp
        src/EEPROM/EEPROM.h
        src/Environment_Sensor/HumidityTempSensor.cpp
        src/Environment_Sensor/HumidityTempSensor.h
        src/Environment_Sensor/PressureSensor.cpp
        src/Environment_Sensor/PressureSensor.h
        src/CO2_Controller/CO2Sensor.cpp
        src/CO2_Controller/CO2Sensor.h
        src/CO2_Controller/FanController.cpp
        src/CO2_Controller/FanController.h
        src/CO2_Controller/CO2Valve.cpp
        src/CO2_Controller/CO2Valve.h
        src/CO2_Controller/CO2Controller.cpp
        src/CO2_Controller/CO2Controller.h
        src/Modbus/SafeModbusClient.cpp
        src/Modbus/SafeModbusClient.h
        src/Modbus/SafeModbusRegister.cpp
        src/Modbus/SafeModbusRegister.h
        # Include the I2C driver that EEPROM depends on
        rp2040-freertos/src/i2c/PicoI2C.cpp
        rp2040-freertos/src/i2c/PicoI2C.h
        rp2040-freertos/src/uart/PicoOsUart.cpp
        rp2040-freertos/src/modbus/ModbusClient.cpp
        rp2040-freertos/src/modbus/ModbusRegister.cpp
        rp2040-freertos/src/modbus/nanomodbus.c
        # Include the mutex wrapper
        rp2040-freertos/src/Fmutex.cpp
        rp2040-freertos/src/Fmutex.h

        ##################network############################
        src/network/Tools/json/Json_handler.cpp
        src/network/Tools/json/Json_handler.h
        src/network/Tools/json/core_json.c
        src/network/entry/thing_speak.h
        src/network/service/thing_speak_service.cpp
        src/network/service/thing_speak_service.h

        rp2040-freertos/src/ipstack/IPStack.h
        rp2040-freertos/src/ipstack/mbedtls_config.h
        rp2040-freertos/src/ipstack/lwipopts_tls.h
        rp2040-freertos/src/ipstack/lwipopts.h
        rp2040-freertos/src/ipstack/tls_common.c
        rp2040-freertos/src/FreeRTOSConfig.h
        ##################network############################
)

# Set include directories for the library
# Anyone who links to this library will automatically get these include paths
target_include_directories(${PROJECT_NAME}_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/modbus
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/i2c
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/display
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/uart
        ${CMAKE_CURRENT_SOURCE_DIR}/rp2040-freertos/src/ipstack
        ${CMAKE_CURRENT_SOURCE_DIR}/src/network
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Tools
)

# Link libraries that the library itself needs
target_link_libraries(${PROJECT_NAME}_lib
        pico_stdlib
        hardware_pwm
        hardware_gpio
        hardware_uart
        hardware_i2c
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap4
        ##################network############################
        pico_cyw43_arch_lwip_sys_freertos
        pico_mbedtls
        pico_lwip_mbedtls
        pico_lwip
        ##################network############################
)

# Now create the main executable with just the main.cpp
# It will use all the code from the library
add_executable(${PROJECT_NAME}
        src/main.cpp
)

# Link the main executable to the library
target_link_libraries(${PROJECT_NAME}
        ${PROJECT_NAME}_lib
)

# Create map/bin/hex/uf2 files for main executable
pico_add_extra_outputs(${PROJECT_NAME})

# Disable usb output, enable uart output for main executable
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)

# Add the test subdirectory
# This will process the CMakeLists.txt in the test folder
add_subdirectory(test)
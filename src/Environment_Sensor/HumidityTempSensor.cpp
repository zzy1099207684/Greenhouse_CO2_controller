//
// Created by zbinc on 21.9.2025.
//

#include "HumidityTempSensor.h"

#define UART_NR 1
#define UART_BAUDRATE 19200
#define UART_TX 4
#define UART_RX 5
#define STOP_BITS 2
#define MODBUS_SERVER_ADDR 241
#define RH_REGISTER_ADDR 256
#define TEMP_REGISTER_ADDR 257

HumidityTempSensor::HumidityTempSensor()
    : uart(std::make_shared<PicoOsUart>(UART_NR, UART_TX, UART_RX, UART_BAUDRATE, STOP_BITS)),
      modbus_client(std::make_shared<ModbusClient>(uart)),
      temp_register(modbus_client, MODBUS_SERVER_ADDR, TEMP_REGISTER_ADDR, true),
      humidity_register(modbus_client, MODBUS_SERVER_ADDR, RH_REGISTER_ADDR, true) {
}

float HumidityTempSensor::readTemperature() {
    const uint16_t raw = temp_register.read();
    const auto signed_value = static_cast<int16_t>(raw);
    return signed_value / TEMP_SCALE;
}

float HumidityTempSensor::readHumidity() {
    const uint16_t raw = humidity_register.read();
    return raw / HUMIDITY_SCALE;
}

void HumidityTempSensor::readBoth(float& temperature, float& humidity) {
    temperature = readTemperature();
    humidity = readHumidity();
}

//
// Created by zbinc on 21.9.2025.
//

#include "HumidityTempSensor.h"

#define MODBUS_SERVER_ADDR 241
#define RH_REGISTER_ADDR 256
#define TEMP_REGISTER_ADDR 257

HumidityTempSensor::HumidityTempSensor(const std::shared_ptr<ModbusClient>& modbus_client)
    :modbus_client(modbus_client),
      temp_register(modbus_client, MODBUS_SERVER_ADDR, TEMP_REGISTER_ADDR, true),
      humidity_register(modbus_client, MODBUS_SERVER_ADDR, RH_REGISTER_ADDR, true) {
    temperature = 0.0f;
    humidity = 0.0f;
}

float HumidityTempSensor::readTemperature() {
    const uint16_t raw = temp_register.read();
    temperature = raw / TEMP_SCALE;
    return temperature;
}

float HumidityTempSensor::readHumidity() {
    const uint16_t raw = humidity_register.read();
    humidity = raw / HUMIDITY_SCALE;
    return humidity;
}


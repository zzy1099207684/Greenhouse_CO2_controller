//
// Created by Sheng Tai on 30/09/2025.
//

#include "CO2Sensor.h"

#include <cstring>
#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"

CO2Sensor::CO2Sensor(const std::shared_ptr<SafeModbusClient>& safe_modbus_client)
    : safe_modbus_client(safe_modbus_client),
      co2_register_low(safe_modbus_client, SERVER_ADDR, CO2_REGISTER_ADDR_LOW, true),
      co2_register_high(safe_modbus_client, SERVER_ADDR, CO2_REGISTER_ADDR_HIGH, true)
{
}

float CO2Sensor::getCO2Level()
{
    const uint16_t co2_value_low = co2_register_low.read();
    const uint16_t co2_value_high = co2_register_high.read();
    const uint32_t co2_value_raw_bits = (static_cast<uint32_t>(co2_value_high) << 16) | co2_value_low;

    float co2_value;
    std::memcpy(&co2_value, &co2_value_raw_bits, sizeof(float));
    return co2_value;
}

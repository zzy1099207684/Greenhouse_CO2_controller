//
// Created by Sheng Tai on 30/09/2025.
//

#ifndef GREENHOUSE_CO2_CO2SENSOR_H
#define GREENHOUSE_CO2_CO2SENSOR_H

#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"

class CO2Sensor
{
public:
    explicit CO2Sensor(const std::shared_ptr<SafeModbusClient>& safe_modbus_client);
    [[nodiscard]] float getCO2Level();

private:
    static constexpr int SERVER_ADDR = 240;
    // CO2 level is a 32-bit float, using two registers
    static constexpr int CO2_REGISTER_ADDR_LOW = 0;
    static constexpr int CO2_REGISTER_ADDR_HIGH = 1;
    std::shared_ptr<SafeModbusClient> safe_modbus_client;
    SafeModbusRegister co2_register_low;
    SafeModbusRegister co2_register_high;
};


#endif //GREENHOUSE_CO2_CO2SENSOR_H
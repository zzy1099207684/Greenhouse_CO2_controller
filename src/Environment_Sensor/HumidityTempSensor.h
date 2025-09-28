//
// Created by zbinc on 21.9.2025.
//

#ifndef HUMIDITYTEMPSENSOR_H
#define HUMIDITYTEMPSENSOR_H

#include <memory>
#include "../../rp2040-freertos/src/modbus/ModbusClient.h"
#include "../../rp2040-freertos/src/modbus/ModbusRegister.h"

class HumidityTempSensor {
private:
    std::shared_ptr<ModbusClient> modbus_client;
    ModbusRegister temp_register;
    ModbusRegister humidity_register;

    static constexpr float TEMP_SCALE = 10.0f;
    static constexpr float HUMIDITY_SCALE = 10.0f;

public:
    explicit HumidityTempSensor(const std::shared_ptr<ModbusClient>& modbus_client);

    float readTemperature();
    float readHumidity();

};



#endif //HUMIDITYTEMPSENSOR_H

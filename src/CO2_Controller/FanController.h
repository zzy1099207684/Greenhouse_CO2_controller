//
// Created by Sheng Tai on 30/09/2025.
//

#ifndef GREENHOUSE_CO2_FANCONTROLLER_H
#define GREENHOUSE_CO2_FANCONTROLLER_H

#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"

class FanController
{
public:
    explicit FanController(const std::shared_ptr<SafeModbusClient>& safe_modbus_client);
    void setSpeed(int new_speed);
    [[nodiscard]] int getSpeed() const;
    int getCounter();

private:
    static constexpr int SERVER_ADDR = 1;
    static constexpr int SPEED_REGISTER_ADDR = 0;
    static constexpr int COUNTER_REGISTER_ADDR = 4;
    static constexpr int MAX_SPEED = 1000;
    std::shared_ptr<SafeModbusClient> safe_modbus_client;
    SafeModbusRegister speed_register;
    SafeModbusRegister counter_register;
    int speed;
};



#endif //GREENHOUSE_CO2_FANCONTROLLER_H

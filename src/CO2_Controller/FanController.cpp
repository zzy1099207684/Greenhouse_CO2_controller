//
// Created by Sheng Tai on 30/09/2025.
//

#include "FanController.h"
#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"

FanController::FanController(const std::shared_ptr<SafeModbusClient>& safe_modbus_client)
    : safe_modbus_client(safe_modbus_client),
      speed_register(safe_modbus_client, SERVER_ADDR, SPEED_REGISTER_ADDR, true),
      counter_register(safe_modbus_client, SERVER_ADDR, COUNTER_REGISTER_ADDR, false),
      speed(0)
{
}

void FanController::setSpeed(int new_speed)
{
    if (new_speed < 0)
    {
        new_speed = 0;
    }
    if (new_speed > MAX_SPEED)
    {
        new_speed = MAX_SPEED;
    }
    speed_register.write(new_speed);
    speed = new_speed;
}

int FanController::getSpeed() const
{
    return speed;
}

int FanController::getCounter()
{
    return counter_register.read();
}

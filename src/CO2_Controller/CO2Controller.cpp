//
// Created by Sheng Tai on 30/09/2025.
//

#include "CO2Controller.h"
#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "CO2Valve.h"
#include "CO2Sensor.h"
#include "FanController.h"
#include <cstdio>

CO2Controller::CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client)
    : valve(),
      sensor(safe_modbus_client),
      fan(safe_modbus_client),
      target_co2_level(800.0f) // default target CO2 level
{
    // Create timers
    fan_timer = xTimerCreate("FanTimer", pdMS_TO_TICKS(MAX_FAN_TIME), pdFALSE, this, fanTimerCallback);
    valve_timer = xTimerCreate("ValveTimer", pdMS_TO_TICKS(MAX_VALVE_TIME), pdFALSE, this, valveTimerCallback);
}

void CO2Controller::setTargetCO2Level(float ppm)
{
    target_co2_level = ppm;
}

float CO2Controller::getTargetCO2Level() const
{
    return target_co2_level;
}

float CO2Controller::getCurrentCO2Level()
{
    return sensor.getCO2Level();
}

float CO2Controller::getFanSpeed() const
{
    return static_cast<float>(fan.getSpeed()) / MAX_FAN_SPEED * 100.0f; // return as percentage
}

void CO2Controller::start()
{
    // xTaskCreate(controlTask, "CO2ControlTask", 2048, this, tskIDLE_PRIORITY + 1, nullptr);

}

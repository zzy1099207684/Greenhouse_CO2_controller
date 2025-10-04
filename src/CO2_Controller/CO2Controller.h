//
// Created by Sheng Tai on 30/09/2025.
//

#ifndef GREENHOUSE_CO2_CO2CONTROLLER_H
#define GREENHOUSE_CO2_CO2CONTROLLER_H

#include <memory>
#include "CO2Valve.h"
#include "CO2Sensor.h"
#include "FanController.h"
#include "timers.h"

class CO2Controller
{
public:
    explicit CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client);
    void setTargetCO2Level(float ppm);
    [[nodiscard]] float getTargetCO2Level() const;
    [[nodiscard]] float getCurrentCO2Level();
    [[nodiscard]] float getFanSpeed() const;
    void start();

private:
    static constexpr float DEADBAND = 10.0f; // ppm
    // valve parameters
    static constexpr float VALVE_TIME_SLOPE = 10.0f; // ms/ppm
    static constexpr int MAX_VALVE_TIME = 2000; // ms
    // fan parameters
    static constexpr float FAN_TIME_SLOPE = 5.0f; // ms/ppm
    static constexpr int MAX_FAN_TIME = 2000; // ms
    static constexpr int MAX_FAN_SPEED = 1000; // 1000 = 100%
    // hardware
    CO2Valve valve;
    CO2Sensor sensor;
    FanController fan;
    // timers
    TimerHandle_t fan_timer;
    TimerHandle_t valve_timer;
    static void fanTimerCallback(TimerHandle_t xTimer);
    static void valveTimerCallback(TimerHandle_t xTimer);
    // control task
    static void controlTask(void* pvParameters);
    // parameters
    float target_co2_level; // in ppm
};


#endif //GREENHOUSE_CO2_CO2CONTROLLER_H

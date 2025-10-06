//
// Created by Sheng Tai on 30/09/2025.
//

#ifndef GREENHOUSE_CO2_CO2CONTROLLER_H
#define GREENHOUSE_CO2_CO2CONTROLLER_H

#include <memory>
#include "CO2Valve.h"
#include "CO2Sensor.h"
#include "event_groups.h"
#include "FanController.h"
#include "semphr.h"

class CO2Controller
{
public:
    explicit CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client, EventGroupHandle_t event_group);
    bool setTargetCO2Level(float ppm);
    [[nodiscard]] float getTargetCO2Level() const;
    [[nodiscard]] float getCurrentCO2Level();
    [[nodiscard]] float getFanSpeed() const;
    void start();

private:
    // tasks parameters
    static constexpr int CONTROL_TASK_STACK_SIZE = 2048; // bytes
    static constexpr int CONTROL_TASK_PRIORITY = 3;
    static constexpr int EMERGENCY_TASK_STACK_SIZE = 2048; // bytes
    static constexpr int EMERGENCY_TASK_PRIORITY = 4;
    static constexpr int CONTROL_INTERVAL_MS = 1000; // ms
    static constexpr int EMERGENCY_CHECK_INTERVAL_MS = 500; // ms
    // CO2 level parameters
    static constexpr float CO2_TARGET_MAX = 1500.0f; // ppm
    static constexpr float CO2_CRITICAL = 2000.0f; // ppm
    static constexpr float DEADBAND = 10.0f; // ppm, to prevent rapid toggling
    // valve parameters
    // open time (ms) = K * diff + B
    static constexpr float VALVE_TIME_K = 13.0f;
    static constexpr float VALVE_TIME_B = 20.0f;
    static constexpr int VALVE_MAX_OPEN_TIME = 2000; // ms
    static constexpr int VALVE_WAIT_TIME = 5 * 1000; // ms
    // fan parameters
    // speed (percentage) = K * diff + B
    static constexpr float FAN_SPEED_K = 5.0f;
    static constexpr float FAN_SPEED_B = 5.0f;
    static constexpr int MAX_FAN_SPEED = 1000; // 1000 = 100%
    // hardware
    CO2Valve valve;
    CO2Sensor sensor;
    FanController fan;
    // task
    static void controlTask(void* pvParameters);
    static void emergencyMonitorTask(void* pvParameters);
    // parameters
    float target_co2_level; // in ppm
    bool emergency_state;
    EventGroupHandle_t event_group;
};


#endif //GREENHOUSE_CO2_CO2CONTROLLER_H

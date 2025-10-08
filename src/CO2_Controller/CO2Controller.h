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

class CO2Controller
{
public:
    explicit CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client, EventGroupHandle_t event_group);
    bool setCO2Setpoint(float ppm);
    [[nodiscard]] float getCO2Setpoint() const;
    [[nodiscard]] float getCO2Value() const;
    [[nodiscard]] float getFanSpeed() const;

private:
    // tasks parameters
    static constexpr int TASK_STACK_SIZE = 2048; // bytes
    static constexpr int TASK_PRIORITY = 4;
    static constexpr int CONTROL_INTERVAL_MS = 1000; // ms
    // CO2 level parameters
    static constexpr int CO2_SETPOINT_MAX = 1500; // ppm
    static constexpr int CO2_SETPOINT_MIN = 200; // ppm
    static constexpr int CO2_CRITICAL = 2000; // ppm
    static constexpr int DEADBAND = 50; // ppm, to prevent rapid toggling
    static constexpr int VENTILATION_THRESHOLD = 200; // used only in active ventilation version
    // valve parameters
    // open time (ms) = K * diff, with MIN and MAX limits
    static constexpr int VALVE_OPEN_TIME_K = 13;
    static constexpr int VALVE_OPEN_TIME_MIN = 50; // ms
    static constexpr int VALVE_OPEN_TIME_MAX = 2000; // ms
    static constexpr int MIXING_TIME = 60 * 1000; // ms, can be smaller for a quick demo
    // fan parameters
    // speed (1-1000) = K * diff, with MIN and MAX limits
    static constexpr int FAN_SPEED_K = 50;
    static constexpr int FAN_SPEED_MIN = 125; // 0 - 1000. Though it accepts 0-1000, below 125 the fan won't really spin
    static constexpr int FAN_SPEED_MAX = 1000; // 1000 = full speed
    // hardware
    CO2Valve valve;
    CO2Sensor sensor;
    FanController fan;
    // tasks and timers
    static void controlTask(void* pvParameters);
    EventGroupHandle_t event_group;
    TimerHandle_t closeValveTimerHandle;
    TimerHandle_t mixingTimerHandle;
    static void closeValveTimerCallback(TimerHandle_t xTimer);
    static void mixingTimerCallback(TimerHandle_t xTimer);
    // other
    float CO2_value;
    float CO2_setpoint; // in ppm
    enum CO2ControllerState
    {
        IDLE,
        VENTILATING, // only used in WITH_ACTIVE_VENTILATION version. Won't hurt to have it here always.
        INJECTING,
        MIXING,
        EMERGENCY
    };
    CO2ControllerState controller_state;
};


#endif //GREENHOUSE_CO2_CO2CONTROLLER_H

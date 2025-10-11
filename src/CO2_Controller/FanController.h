//
// Created by Sheng Tai on 30/09/2025.
//

#ifndef GREENHOUSE_CO2_FANCONTROLLER_H
#define GREENHOUSE_CO2_FANCONTROLLER_H

#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "event_groups.h"

class FanController
{
public:
    explicit FanController(const std::shared_ptr<SafeModbusClient>& safe_modbus_client, EventGroupHandle_t event_group);
    void setSpeed(int new_speed);
    [[nodiscard]] int getSpeed() const;

private:
    static constexpr int SERVER_ADDR = 1;
    static constexpr int SPEED_REGISTER_ADDR = 0;
    static constexpr int COUNTER_REGISTER_ADDR = 4;
    static constexpr int MAX_SPEED = 1000;
    static constexpr int MIN_WORKING_SPEED = 125; // below this speed, fan won't really spin
    static constexpr int FAN_CHECK_INTERVAL_MS = 2000; // check every 3 second
    static constexpr int FAN_FAILURE_THRESHOLD = 3; // 3 consecutive zeros means fan is broken
    std::shared_ptr<SafeModbusClient> safe_modbus_client;
    SafeModbusRegister speed_register;
    SafeModbusRegister counter_register;
    EventGroupHandle_t event_group;
    TimerHandle_t fan_check_timer;
    int speed;
    int zero_count; // count consecutive counter reading which is 0
    static void fanCheckTimerCallback(TimerHandle_t xTimer);
};



#endif //GREENHOUSE_CO2_FANCONTROLLER_H

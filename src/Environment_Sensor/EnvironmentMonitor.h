//
// Created by zbinc on 28.9.2025.
//

#ifndef ENVIRONMENTMONITOR_H
#define ENVIRONMENTMONITOR_H


#include "PressureSensor.h"
#include "HumidityTempSensor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "event_groups.h"


class EnvironmentMonitor {
private:
    //PressureSensor& pressure_sensor;
    HumidityTempSensor& humidity_temp_sensor;
    EventGroupHandle_t controller_event_group;
    TimerHandle_t timer_handle;

    static constexpr uint32_t INTERVAL_MS = 10000;

    static void timer_callback(TimerHandle_t xTimer);

    void readAndSendData() const;

public:
    struct SensorData {
        float temperature;
        float humidity;
    };

    explicit EnvironmentMonitor(HumidityTempSensor& humidity_temp_sen);
    void setEventGroup(EventGroupHandle_t event_group);
    void start();
    void stop() const;
    ~EnvironmentMonitor();
};
#endif //ENVIRONMENTMONITOR_H

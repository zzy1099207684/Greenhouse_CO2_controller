//
// Created by zbinc on 28.9.2025.
//

#ifndef ENVIRONMENTMONITOR_H
#define ENVIRONMENTMONITOR_H


#include "PressureSensor.h"
#include "HumidityTempSensor.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


class EnvironmentMonitor {
private:
    PressureSensor& pressure_sensor;
    HumidityTempSensor& humidity_temp_sensor;
    QueueHandle_t data_queue;
    TimerHandle_t timer_handle;

    static constexpr uint32_t INTERVAL_MS = 10000;

    static void timer_callback(TimerHandle_t xTimer);

    void readAndSendData();

public:
    struct SensorData {
        float pressure;
        float temperature;
        float humidity;
    };

    explicit EnvironmentMonitor(PressureSensor& pressure_sen, HumidityTempSensor& humidity_temp_sen);
    void setQueue(QueueHandle_t queue);
    void start();
    void stop();
    ~EnvironmentMonitor();
};
#endif //ENVIRONMENTMONITOR_H

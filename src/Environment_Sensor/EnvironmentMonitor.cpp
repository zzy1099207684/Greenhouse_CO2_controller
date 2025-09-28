//
// Created by zbinc on 28.9.2025.
//

#include "EnvironmentMonitor.h"

EnvironmentMonitor::EnvironmentMonitor(
    PressureSensor &pressure_sen, HumidityTempSensor &humidity_temp_sen
    ):pressure_sensor(pressure_sen),
    humidity_temp_sensor(humidity_temp_sen),
    data_queue(nullptr),
    timer_handle(nullptr){}

EnvironmentMonitor::~EnvironmentMonitor() {
    if (timer_handle != nullptr) {
        xTimerDelete(timer_handle, 0);
    }
}

void EnvironmentMonitor::setQueue(QueueHandle_t queue) {
    data_queue = queue;
}


void EnvironmentMonitor::start() {
    if (timer_handle == nullptr) {
        timer_handle = xTimerCreate(
            "EnvMonitorTimer",
            pdMS_TO_TICKS(INTERVAL_MS),
            pdTRUE,
            this,
            timer_callback
        );
    }
    if (timer_handle != nullptr) xTimerStart(timer_handle, 0);
}

void EnvironmentMonitor::stop() {
    if (timer_handle != nullptr) xTimerStop(timer_handle, 0);
}

void EnvironmentMonitor::timer_callback(TimerHandle_t xTimer) {
    auto* monitor = static_cast<EnvironmentMonitor*>(pvTimerGetTimerID(xTimer));
    monitor->readAndSendData();
}

void EnvironmentMonitor::readAndSendData() {
    SensorData data{};
    data.pressure = pressure_sensor.getPressure();
    data.temperature = humidity_temp_sensor.readTemperature();
    data.humidity = humidity_temp_sensor.readHumidity();
    xQueueSend(data_queue, &data, portMAX_DELAY);
}


//
// Created by zbinc on 28.9.2025.
//

#include "EnvironmentMonitor.h"

EnvironmentMonitor::EnvironmentMonitor(HumidityTempSensor &humidity_temp_sensor
    ):humidity_temp_sensor(humidity_temp_sensor),
    controller_event_group(nullptr),
    timer_handle(nullptr){}

EnvironmentMonitor::~EnvironmentMonitor() {
    if (timer_handle != nullptr) {
        xTimerDelete(timer_handle, 0);
    }
}

void EnvironmentMonitor::setEventGroup(EventGroupHandle_t event_group) {
    controller_event_group = event_group;
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

void EnvironmentMonitor::stop() const {
    if (timer_handle != nullptr) xTimerStop(timer_handle, 0);
}

void EnvironmentMonitor::timer_callback(TimerHandle_t xTimer) {
    auto* monitor = static_cast<EnvironmentMonitor*>(pvTimerGetTimerID(xTimer));
    monitor->readAndSendData();
}

void EnvironmentMonitor::readAndSendData() const {
    SensorData data{};
    data.temperature = humidity_temp_sensor.readTemperature();
    data.humidity = humidity_temp_sensor.readHumidity();

}


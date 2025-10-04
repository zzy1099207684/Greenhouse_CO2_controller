//
// Created by zbinc on 21.9.2025.
//

#include "GreenhouseMonitor.h"

GreenhouseMonitor::GreenhouseMonitor(HumidityTempSensor &humidityTempSensor)
    :humidityTempSensor(humidityTempSensor){

}

void GreenhouseMonitor::sensor_timer_callback(TimerHandle_t xTimer) {
    auto* monitor = static_cast<GreenhouseMonitor*>(pvTimerGetTimerID(xTimer));
    monitor->read_sensor_data();
}

void GreenhouseMonitor::read_sensor_data() {
    systemData.humidity = humidityTempSensor.readHumidity();
    systemData.temperature = humidityTempSensor.readTemperature();
    printf("Humidity: %f\n", systemData.humidity);
    printf("Temperature: %f\n", systemData.temperature);
    /*
     * TODO:
     * 1. read co2
     * 2. read fan speed
     * 3. call ui & network setters
     */
}

void GreenhouseMonitor::sensor_timer_start() {
    if (sensor_timer_handle == nullptr) {
        sensor_timer_handle = xTimerCreate(
            "EnvMonitorTimer",
            pdMS_TO_TICKS(INTERVAL_MS),
            pdTRUE,
            this,
            sensor_timer_callback
        );
    }
    if (sensor_timer_handle != nullptr) xTimerStart(sensor_timer_handle, 0);
}

void GreenhouseMonitor::init() {
    sensor_timer_start();
    /*
     * TODO:
     * 1. pass event group handle to member classes
     * 2. create and start monitor task
     * 3. start other task if needed
     * 4. load setting from eeprom
     */
}




//
// Created by zbinc on 21.9.2025.
//

#include "GreenhouseMonitor.h"

GreenhouseMonitor::GreenhouseMonitor(HumidityTempSensor &humidityTempSensor, thing_speak& ts)
    :humidityTempSensor(humidityTempSensor), ts(ts){
    monitor_event_group = nullptr;
}

void GreenhouseMonitor::network_connection() {
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(monitor_event_group,
        UI_GET_NETWORK|UI_CONNECT_NETWORK,
        true, false, portMAX_DELAY);
        if (bits & UI_GET_NETWORK) {
            thing_speak_service::scan_wifi_ssid_arr(ts);
            auto ssids = ts.get_wifi_scan_result();
            // just for test
            for (int i = 0; i < 9; i++) {
                if (ssids[i][0] != '\0') {
                    printf("%s\n", ssids[i]);
                }
            }
        }
        //if (bits & UI_CONNECT_NETWORK) {
        //set ssid, set password, save to eeprom, connect wifi
        //}
    }
}

void GreenhouseMonitor::network_connection_task(void *pvParameters) {
    auto* monitor = static_cast<GreenhouseMonitor*>(pvParameters);
    monitor->network_connection();
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
    monitor_event_group = xEventGroupCreate();
    thing_speak_service::wifi_init();
    xTaskCreate(&network_connection_task, "network_connection_task", 2048, nullptr, , nullptr);
    sensor_timer_start();
    /*
     * TODO:
     * 1. pass event group handle to member classes
     * 2. create and start monitor task
     * 3. start other task if needed
     * 4. load setting from eeprom
     */
}




//
// Created by zbinc on 21.9.2025.
//

#include "GreenhouseMonitor.h"

GreenhouseMonitor::GreenhouseMonitor(thing_speak& ts, thing_speak_service& ts_service):
    uart(std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2)),
    modbus_client(std::make_shared<SafeModbusClient>(uart)),
    i2c0bus(std::make_shared<PicoI2C>(0, 400000)),
    i2c1bus(std::make_shared<PicoI2C>(1, 400000)),
    monitor_event_group(xEventGroupCreate()),
    co2_controller(modbus_client),
    ui(i2c1bus, monitor_event_group),
    humidityTempSensor(modbus_client),
    eeprom(i2c0bus),
    ts(ts), ts_service(ts_service){
    ts.set_co2_wifi_scan_event_group(monitor_event_group);
}

void GreenhouseMonitor::network_init() const {
    printf("wifi_init start\n");
    ts_service.network_init(&ts);
    vTaskDelete(nullptr);
}

void GreenhouseMonitor::network_init_task(void *pvParameters) {
    auto* monitor = static_cast<GreenhouseMonitor*>(pvParameters);
    monitor->network_init();
}


void GreenhouseMonitor::network_connection(){
    while (1) {
        EventBits_t bits = xEventGroupWaitBits(monitor_event_group,
        UI_GET_NETWORK|UI_CONNECT_NETWORK,
        true, false, portMAX_DELAY);;
        if (bits & UI_GET_NETWORK) {
            ts_service.scan_wifi_ssid_arr(&ts);
            //get ssid list and give to ui
            xEventGroupSetBits(monitor_event_group, UI_SSID_READY);
        }
        if (bits & UI_CONNECT_NETWORK) {
        //set ssid, set password, save to eeprom, connect wifi
            //strcpy(ssid, ui.get_ssid());
            //strcpy(pwd, ui.get_password());
            // eeprom.writeSSID(ssid);
            // eeprom.writePWD(pwd);
            // ts.set_ssid(ssid);
            // ts.set_pwd(pwd);
            // thing_speak_service::wifi_connect(&ts);
        }
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
    systemData.co2Level = static_cast<int>(co2_controller.getCurrentCO2Level());
    systemData.fanSpeed = static_cast<int>(co2_controller.getFanSpeed());

    ts.set_Temperature(systemData.temperature);
    ts.set_Relative_humidity(systemData.humidity);
    ts.set_fan_speed(systemData.fanSpeed);
    ts.set_CO2_level(systemData.co2Level);

    ui.set_Temperature(systemData.temperature);
    ui.set_Relative_humidity(systemData.humidity);
    ui.set_fan_speed(systemData.fanSpeed);
    ui.set_CO2_level(systemData.co2Level);

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


void GreenhouseMonitor::greenhouse_monitor_task() {
    while (1) {
        auto bit = xEventGroupWaitBits(monitor_event_group,
            UI_SET_CO2 | NETWORK_SET_CO2| CO2_WARNING,
            pdTRUE, pdFALSE, portMAX_DELAY);
        if (bit&UI_SET_CO2) {
            //systemData.co2SetPoint = ui.get_CO2_level();
            co2_controller.setTargetCO2Level(static_cast<float>(systemData.co2SetPoint));
            printf("setting from ui");
        }
        if (bit&NETWORK_SET_CO2) {
            systemData.co2SetPoint = ts.get_co2_level_from_network();
            co2_controller.setTargetCO2Level(static_cast<float>(systemData.co2Level));
            ui.set_CO2_level(systemData.co2SetPoint);
            //eeprom.writeCO2Value(systemData.co2SetPoint);
        }
        if (bit&CO2_WARNING) {
            printf("warning");
        }
    }

}

void GreenhouseMonitor::greenhouse_monitor_run(void *pvParameters) {
    auto* monitor = static_cast<GreenhouseMonitor*>(pvParameters);
    monitor->greenhouse_monitor_task();
}


void GreenhouseMonitor::init() {
    // eeprom.readSSID(ssid);
    // eeprom.readPWD(pwd);
    // eeprom.writeCO2Value(450);
    // eeprom.readCO2Value(systemData.co2SetPoint);
    //co2_controller.setTargetCO2Level(systemData.co2Level);
    ts.set_ssid("B38-2G");
    ts.set_pwd("kisupupu8697");


    xTaskCreate(network_init_task, "network_init_task", 1024, this, tskIDLE_PRIORITY+2, nullptr);
    xTaskCreate(thing_speak_service::start, "thing_speak_service_start", 1024, &ts, tskIDLE_PRIORITY + 1, nullptr);
    xTaskCreate(network_connection_task, "network_connection_task", 1024, this, tskIDLE_PRIORITY + 2, nullptr);
    xTaskCreate(greenhouse_monitor_run, "greenhouse_monitor_run", 1024, this, tskIDLE_PRIORITY + 2, nullptr);
    co2_controller.start();
    sensor_timer_start();
    /*
     * TODO:
     * 1. pass event group handle to member classes
     * 2. create and start monitor task
     * 3. start other task if needed
     * 4. load setting from eeprom
     */
}






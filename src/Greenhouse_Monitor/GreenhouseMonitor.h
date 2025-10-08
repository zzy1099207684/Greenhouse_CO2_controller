//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSEMONITOR_H
#define GREENHOUSEMONITOR_H

#include "pico/cyw43_arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "EEPROM/EEPROM.h"
#include "Environment_Sensor/HumidityTempSensor.h"
//#include "Environment_Sensor/PressureSensor.h"
#include "network/entry/thing_speak.h"
#include "network/service/thing_speak_service.h"
#include "CO2_Controller/CO2Controller.h"
#include "ui/ui.h"

#define UI_SET_CO2 (1<<0) //co2 setting ready from ui
#define UI_GET_NETWORK (1<<1) //ui needs ssid list
#define UI_SSID_READY (1<<2) //set by controller, tell ui ssid list ready
#define UI_CONNECT_NETWORK (1<<3) //password ready from ui


#define NETWORK_SET_CO2 (1<<4) //co2 setting from network
#define WIFI_INIT (1 << 5) // wifi_init_success
#define WIFI_SCAN_DONE (1 << 6) // wifi_scan_done

#define CO2_WARNING (1<<7) //warning from co2 controller, co2 critical high level

#define ENV_SENSOR_TIMER_REACHED (1<<8)

#define WIFI_CONNECTED (1<<9)

#define FAN_WARNING (1<<12) //warning from fan controller, fan is not working



class GreenhouseMonitor {
private:

    std::shared_ptr<PicoOsUart> uart;
    std::shared_ptr<SafeModbusClient> modbus_client;
    std::shared_ptr<PicoI2C> i2c0bus;
    std::shared_ptr<PicoI2C> i2c1bus;

    CO2Controller co2_controller;
    UI_control ui;

    HumidityTempSensor humidityTempSensor;
    //PressureSensor& pressureSensor;
    EEPROM eeprom;
    thing_speak& ts;
    thing_speak_service& ts_service;

    struct {
        int co2Level;
        float temperature;
        float humidity;
        int fanSpeed;
        int co2SetPoint;
    } systemData{};

    xSemaphoreHandle system_data_mutex;

    char ssid[64]{};
    char pwd[64]{};

    TimerHandle_t sensor_timer_handle = nullptr;
    static constexpr uint32_t INTERVAL_MS = 1000; // for demo purpose, set to 1 second, fast
    static constexpr int CO2_SETPOINT_MAX = 1500; // ppm
    static constexpr int CO2_SETPOINT_MIN = 200; // ppm
    static constexpr int CO2_SETPOINT_DEFAULT = 800; // ppm
    EventGroupHandle_t monitor_event_group;

    static void sensor_timer_callback(TimerHandle_t xTimer);
    void notify_sensors() const;
    void read_sensor_task();
    static void read_sensor_run(void *pvParameters);
    void sensor_timer_start();

    void network_setting();
    static void network_setting_task(void *pvParameters);

    void network_init() const;
    static void network_init_task(void *pvParameters);

    void greenhouse_monitor_task();
    static void greenhouse_monitor_run(void *pvParameters);



public:
    GreenhouseMonitor(thing_speak& ts, thing_speak_service& ts_service);
    //~GreenhouseMonitor();

    void init();

};



#endif //GREENHOUSEMONITOR_H

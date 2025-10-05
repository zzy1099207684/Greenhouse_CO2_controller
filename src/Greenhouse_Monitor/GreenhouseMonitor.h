//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSEMONITOR_H
#define GREENHOUSEMONITOR_H

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "EEPROM/EEPROM.h"
#include "Environment_Sensor/HumidityTempSensor.h"
//#include "Environment_Sensor/PressureSensor.h"
#include "network/entry/thing_speak.h"
#include "network/service/thing_speak_service.h"
#include "CO2_Controller/CO2Controller.h"

#define UI_SET_CO2 (1<<0) //co2 setting ready from ui
#define UI_GET_NETWORK (1<<1) //ui needs ssid list
#define UI_SSID_READY (1<<2) //set by controller, tell ui ssid list ready
#define UI_CONNECT_NETWORK (1<<3) //password ready from ui


#define NETWORK_SET_CO2 (1<<4) //co2 setting from network
#define WIFI_INIT (1 << 5) // wifi_init_success
#define WIFI_SCAN_DONE (1 << 6) // wifi_scan_done

#define CO2_WARNING (1<<7) //warning from co2 controller



class GreenhouseMonitor {
private:
    CO2Controller& co2_controller;
    //UI& ui;
    thing_speak& ts;
    thing_speak_service& ts_service;
    HumidityTempSensor& humidityTempSensor;
    //PressureSensor& pressureSensor;
    EEPROM& eeprom;

    struct {
        int co2Level;
        float temperature;
        float humidity;
        int fanSpeed;
        int co2SetPoint;
    } systemData{};

    char ssid[64]{};
    char pwd[64]{};

    TimerHandle_t sensor_timer_handle = nullptr;
    static constexpr uint32_t INTERVAL_MS = 1000;

    EventGroupHandle_t monitor_event_group;

    static void sensor_timer_callback(TimerHandle_t xTimer);
    void read_sensor_data();
    void sensor_timer_start();

    void network_connection() const;
    static void network_connection_task(void *pvParameters);

    void network_init() const;
    static void network_init_task(void *pvParameters);

    void greenhouse_monitor_task();
    static void greenhouse_monitor_run(void *pvParameters);



public:
    GreenhouseMonitor(CO2Controller& co2_controller, EEPROM& eeprom, HumidityTempSensor& humidityTempSensor, thing_speak& ts, thing_speak_service& ts_service);
    //~GreenhouseMonitor();

    void init();

};



#endif //GREENHOUSEMONITOR_H

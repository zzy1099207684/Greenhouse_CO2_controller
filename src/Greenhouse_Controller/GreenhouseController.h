//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSECONTROLLER_H
#define GREENHOUSECONTROLLER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "EEPROM/EEPROM.h"
#include "Environment_Sensor/EnvironmentMonitor.h"


typedef struct {
    uint16_t co2Level;
    float temperature;
    float humidity;
    float pressure;
    uint16_t co2SetPoint;
} SystemData_t;

class GreenhouseController {
private:
    // Co2Controller& co2_controller;
    // DisplayManager& display_manager;
    // NetworkManager& network_manager;
    EEPROM& eeprom;
    EnvironmentMonitor& environmentMonitor;

    QueueHandle_t co2_setting_q;
    QueueHandle_t co2_reading_q;
    QueueHandle_t environment_reading_q;

    SystemData_t system_data;

    static void waiting_co2_reading(void* pvParameters);
    static void waiting_environment_reading(void* pvParameters);
    static void waiting_co2_setting(void* pvParameters);


public:
    GreenhouseController(EEPROM& eeprom, EnvironmentMonitor& environmentMonitor);
    ~GreenhouseController();
};



#endif //GREENHOUSECONTROLLER_H

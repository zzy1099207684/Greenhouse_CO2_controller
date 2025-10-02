//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSECONTROLLER_H
#define GREENHOUSECONTROLLER_H



#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

#include "EEPROM/EEPROM.h"
#include "Environment_Sensor/EnvironmentMonitor.h"

//#define CO2_READING_BIT (1<<0)

#define UI_SET_CO2_BIT (1<<0)
#define UI_GET_NETWORK (1<<1)
#define UI_SET_NETWORK (1<<2)
#define NETWORK_SET_CO2_BIT (1<<3)
#define CO2_WARNING_BIT (1<<4)



class GreenhouseController {
private:
    // Co2Controller& co2_controller;
    // DisplayManager& display_manager;
    // NetworkManager& network_manager;
    EEPROM& eeprom;
    EnvironmentMonitor& environmentMonitor;

    struct {
        uint16_t co2Level;
        float temperature;
        float humidity;
        int fanSpeed;
        uint16_t co2SetPoint;
    } SystemData;

    EventGroupHandle_t controller_event_group; //give event group handle to everybody

    void init(); //create task, event group, give event group handle to everybody etc

    static void controller_task(void* pvParameters);
        /*
         * while(1) {
        *  EventBits_t bit = xEventGroupWaitBits(eventGroup,
            BIT1 | BIT2 | BIT3, ...,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
            if (bit & ...)
         * }
         */


public:
    GreenhouseController(EEPROM& eeprom, EnvironmentMonitor& environmentMonitor);
    ~GreenhouseController();
};



#endif //GREENHOUSECONTROLLER_H

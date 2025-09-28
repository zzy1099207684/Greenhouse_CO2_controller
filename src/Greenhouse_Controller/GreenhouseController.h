//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSECONTROLLER_H
#define GREENHOUSECONTROLLER_H

#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/FreeRTOS.h"
#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/task.h"
#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/queue.h"

#include "EEPROM/EEPROM.h"


typedef struct {
    uint16_t co2Level;
    float temperature;
    float humidity;
    float pressure;
    uint16_t co2SetPoint;
} SystemData_t;

class GreenhouseController {
private:
    Co2Controller& co2_controller;

    HumidityTempSensor& humidity_temp_sensor;
    PressureSensor& pressure_sensor;

    DisplayManager& display_manager;
    NetworkManager& network_manager;
    EEPROM& eeprom;

    QueueHandle_t co2_set_queue;
    QueueHandle_t co2_reading_queue;

    SystemData_t system_data;

    static void controllerTaskFunction(void* pvParameters);

    void updateDisplay();
    void updateNetwork();

    void saveSystemData();
    void loadSystemData();

public:
    GreenhouseController();
    ~GreenhouseController();
    void init(); // call init function of all components, create queues, load data, create greenhouse controller task

};



#endif //GREENHOUSECONTROLLER_H

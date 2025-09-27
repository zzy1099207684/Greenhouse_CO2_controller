//
// Created by zbinc on 21.9.2025.
//

#ifndef GREENHOUSECONTROLLER_H
#define GREENHOUSECONTROLLER_H

#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/FreeRTOS.h"
#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/task.h"
#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/queue.h"
#include "../../rp2040-freertos/FreeRTOS-KernelV10.6.2/include/semphr.h"

// Forward declarations
class Co2Sensor;
class ValveController;
class FanController;
class HumidityTempSensor;
class PressureSensor;
class DisplayManager;
class NetworkManager;
class EEPROM;

typedef struct {
    uint16_t co2Level;
    float temperature;
    float humidity;
    float pressure;
    uint16_t co2SetPoint;
} SystemData_t;

class GreenhouseController {
private:
    Co2Sensor& co2_sensor;
    ValveController& valve_controller;
    FanController& fan_controller;
    // or these are one thing?

    HumidityTempSensor& humidity_temp_sensor;
    PressureSensor& pressure_sensor;

    DisplayManager& display_manager;
    NetworkManager& network_manager;
    EEPROM& eeprom_manager;

    QueueHandle_t display_queue;
    QueueHandle_t network_queue;
    QueueHandle_t eeprom_queue;
    QueueHandle_t command_queue;

    SystemData_t system_data;

    static void controllerTaskFunction(void* pvParameters);
    void getData();//co2, temp, humidity, pressure

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

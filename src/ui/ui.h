//
// Created by Riina on 25/09/2025.
//

#ifndef UI_H
#define UI_H
#include <memory>
#include <queue.h>
#include "FreeRTOS.h"
#include "ssd1306os.h"

class UI_control {
public:
    UI_control(QueueHandle_t queue,std::shared_ptr<PicoI2C> i2cbus);

    int get_CO2_level();

    void set_CO2_level(int CO2_level);
    void set_Relative_humidity(int Relative_humidity);
    void set_Temperature(int Temperature);
    void set_fan_speed(int Fan_speed);

    void update_ui();
    void read_input();
    void input_handle();

private:
    int CO2_level;
    int Relative_humidity;
    int Temperature;
    int fan_speed;
    QueueHandle_t controller_queue;
    ssd1306os display;
};

#endif //UI_H

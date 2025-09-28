//
// Created by zbinc on 27.9.2025.
//

#include "PressureSensor.h"

#include <cmath>
#include <utility>

PressureSensor::PressureSensor(const std::shared_ptr<PicoI2C> &i2c_bus) : i2c(i2c_bus) {
    altitude = 0.0f;
    last_reading = 0.0f;
    i2c->write(ADDRESS, &CMD_RESET, 1);
    vTaskDelay(pdMS_TO_TICKS(60));
}

void PressureSensor::set_altitude(const float altitude) {
    this->altitude = altitude;
}


float PressureSensor::getPressure() {
    uint8_t buffer[3];
    if (i2c->write(ADDRESS, &CMD_TRIGGER, 1) != 1) return 0.0f;
    vTaskDelay(pdMS_TO_TICKS(10));
    if (i2c->read(ADDRESS, buffer, 3) != 3) return 0.0f;
    int16_t raw = buffer[0] << 8 | buffer[1];
    float pressure = static_cast<float>(raw) / SCALE_FACTOR;
    if (altitude > 0) {
        float P_amb = 1013.25f * std::pow(1.0f - 0.0000225577f * altitude, 5.25588f);
        pressure = pressure * (P_CAL / P_amb);
    }
    last_reading = pressure;
    return pressure;
}


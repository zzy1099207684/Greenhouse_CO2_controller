//
// Created by zbinc on 27.9.2025.
//

#ifndef PRESSURESENSOR_H
#define PRESSURESENSOR_H

#include <memory>

#include "PicoI2C.h"

class PressureSensor {
private:
    std::shared_ptr<PicoI2C> i2c;
    static constexpr uint8_t ADDRESS = 0x40;
    static constexpr uint8_t CMD_TRIGGER = 0xF1;
    static constexpr uint8_t CMD_RESET = 0xFE;
    static constexpr float SCALE_FACTOR = 240.0f;
    static constexpr float P_CAL = 966.0f;
    float altitude;
    float last_reading;
public:
    explicit PressureSensor(const std::shared_ptr<PicoI2C> &i2c_bus);
    void set_altitude(float altitude);
    [[nodiscard]] float getPressure();
};



#endif //PRESSURESENSOR_H

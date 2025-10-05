//
// Created by Sheng Tai on 03/10/2025.
//

#ifndef GREENHOUSE_CO2_CO2VALVE_H
#define GREENHOUSE_CO2_CO2VALVE_H
#include "hardware/gpio.h"


class CO2Valve
{
public:
    explicit CO2Valve();
    void open();
    void close();
    [[nodiscard]] bool isOpen() const;


private:
    static constexpr int RELAY_GPIO = 27;
};


#endif //GREENHOUSE_CO2_CO2VALVE_H

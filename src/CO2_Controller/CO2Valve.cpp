//
// Created by Sheng Tai on 03/10/2025.
//

#include "CO2Valve.h"
#include "hardware/gpio.h"

CO2Valve::CO2Valve()
{
    gpio_init(RELAY_GPIO);
    gpio_set_dir(RELAY_GPIO, GPIO_OUT);
    gpio_put(RELAY_GPIO, false); // Ensure the valve is closed initially
}

void CO2Valve::open()
{
    gpio_put(RELAY_GPIO, true);
}

void CO2Valve::close()
{
    gpio_put(RELAY_GPIO, false);
}

bool CO2Valve::isOpen() const
{
    return gpio_get(RELAY_GPIO) == 1;
}

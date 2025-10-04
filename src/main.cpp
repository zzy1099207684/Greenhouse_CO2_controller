#include <memory>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "Greenhouse_Monitor/GreenhouseMonitor.h"

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}


int main() {
    stdio_init_all();

    auto uart{std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};

    auto i2c1bus{std::make_shared<PicoI2C>(1, 400000)};
    auto i2c0bus{std::make_shared<PicoI2C>(0, 400000)};
    return 0;
}

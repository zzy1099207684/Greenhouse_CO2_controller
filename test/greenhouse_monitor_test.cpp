//
// Created by zbinc on 2.10.2025.
//

#include <pico/stdio.h>
#include "Greenhouse_Monitor/GreenhouseMonitor.h"
#include "Environment_Sensor/HumidityTempSensor.h"


extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}

int main() {
    stdio_init_all();
    auto uart{std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};
    HumidityTempSensor humidity_temp_Sensor(rtu_client);
    GreenhouseMonitor monitor(humidity_temp_Sensor);
    monitor.init();
    vTaskStartScheduler();

    // Should never reach here
    while (1) {
    }
}

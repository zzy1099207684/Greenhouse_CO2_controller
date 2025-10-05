#include <memory>
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
    auto modbus_client{std::make_shared<SafeModbusClient>(uart)};
    CO2Controller co2_controller(modbus_client);
    HumidityTempSensor humidity_temp_sensor(modbus_client);
    thing_speak ts;
    thing_speak_service ts_service;

    auto i2c1bus{std::make_shared<PicoI2C>(1, 400000)};
    //ui...

    auto i2c0bus{std::make_shared<PicoI2C>(0, 400000)};
    EEPROM  eeprom{i2c0bus};

    GreenhouseMonitor monitor(co2_controller, eeprom, humidity_temp_sensor, ts, ts_service);
    monitor.init();
    vTaskStartScheduler();
    while (1) {}

    return 0;
}

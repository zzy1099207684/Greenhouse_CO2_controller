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

    static thing_speak ts;
    static thing_speak_service ts_service;

    GreenhouseMonitor monitor(ts, ts_service);
    printf("monitor started\n");
    monitor.init();

    vTaskStartScheduler();
    while (1) {}

    return 0;
}

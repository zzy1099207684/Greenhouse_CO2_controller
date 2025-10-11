#include <memory>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "Greenhouse_Monitor/GreenhouseMonitor.h"
#include "Utils/Debug.h"

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }

    // Stack overflow hook - called when FreeRTOS detects stack overflow
    void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
        printf("\n!!! STACK OVERFLOW: Task '%s' !!!\n", pcTaskName);
        while(1) {
            tight_loop_contents();
        }
    }

    // Malloc failed hook
    void vApplicationMallocFailedHook(void) {
        printf("\n!!! MALLOC FAILED !!!\n");
        while(1) {
            tight_loop_contents();
        }
    }
}


int main() {
    stdio_init_all();

    Debug::init();
    thing_speak ts;
    thing_speak_service ts_service;

    GreenhouseMonitor monitor(ts, ts_service);
    printf("monitor started\n");
    monitor.init();

    vTaskStartScheduler();
    while (1) {}

    return 0;
}

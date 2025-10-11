

#include "Environment_Sensor/PressureSensor.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <cstring>
#include <memory>
#include <hardware/structs/timer.h>
#include <pico/stdio.h>

extern "C" {
    uint32_t read_runtime_ctr(void) {
        return timer_hw->timerawl;
    }
}

void test_task(void *pvParameters) {
    auto i2cbus{std::make_shared<PicoI2C>(1, 400000)};
    PressureSensor s(i2cbus);
    printf("Starting Test...\n");


    while (1) {
        // Read raw values
        float pressure = s.getPressure();

        // Display results
        printf("----------------------------------------\n");
        printf("  Pressure:    %.1f%\n", pressure);
        printf("----------------------------------------\n\n");

        // Wait 2 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();

    // Give time for USB serial to connect
    // vTaskDelay(pdMS_TO_TICKS(2000));

    printf("\n\n");
    printf("====================================\n");
    printf("  Pressure Test Program\n");
    printf("====================================\n\n");

    // Create test task
    xTaskCreate(test_task,
                "TestTask",
                1024, // Stack size
                NULL, // Parameters
                1, // Priority
                NULL); // Task handle

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (1) {
    }

    return 0;
}

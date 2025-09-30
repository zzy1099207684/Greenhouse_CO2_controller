//
// Created by zbinc on 30.9.2025.
//

#include "Environment_Sensor/HumidityTempSensor.h"
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
    auto uart{std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2)};
    auto rtu_client{std::make_shared<ModbusClient>(uart)};
    HumidityTempSensor rh_t_sensor{rtu_client};
    printf("Starting HMP Sensor Test...\n");
    printf("Slave Address: 240\n");
    printf("Baud Rate: 9600\n");
    printf("Reading every 2 seconds...\n\n");

    while(1) {
        // Read raw values
        uint16_t humidity_raw = rh_t_sensor.readHumidity();
        uint16_t temp_raw = rh_t_sensor.readTemperature();

        // Convert to actual values (values are scaled by 10)
        float humidity = humidity_raw / 10.0f;

        // Temperature can be negative, so treat as signed
        int16_t temp_signed = static_cast<int16_t>(temp_raw);
        float temperature = temp_signed / 10.0f;

        // Display results
        printf("----------------------------------------\n");
        printf("Raw Values:\n");
        printf("  Humidity:    %u (0x%04X)\n", humidity_raw, humidity_raw);
        printf("  Temperature: %u (0x%04X)\n", temp_raw, temp_raw);
        printf("\nConverted Values:\n");
        printf("  Humidity:    %.1f %%RH\n", humidity);
        printf("  Temperature: %.1f °C\n", temperature);
        printf("----------------------------------------\n\n");

        // Wait 2 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main() {
    // Initialize stdio
    stdio_init_all();

    // Give time for USB serial to connect
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("\n\n");
    printf("====================================\n");
    printf("  HMP Sensor Modbus Test Program\n");
    printf("====================================\n\n");

    // Create test task
    xTaskCreate(test_task,
                "TestTask",
                1024,           // Stack size
                NULL,           // Parameters
                1,              // Priority
                NULL);          // Task handle

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while(1) {
        tight_loop_contents();
    }

    return 0;
}
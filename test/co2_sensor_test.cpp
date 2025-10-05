//
// Created by Sheng Tai on 30/09/2025.
//

#include "CO2_Controller/CO2Sensor.h"
#include "uart/PicoOsUart.h"
#include "Modbus/SafeModbusClient.h"
#include "modbus/ModbusClient.h"
#include "FreeRTOS.h"
#include "task.h"
#include <cstdio>
#include <memory>
#include <pico/stdio.h>
#include "hardware/timer.h"
#include "pico/time.h"

extern "C" {
uint32_t read_runtime_ctr(void)
{
    return timer_hw->timerawl;
}
}

void test_co2_sensor_task(void* params)
{
    printf("=== CO2 Sensor Test Starting ===\n\n");

    // Initialize UART1 on GPIO4 (TX) and GPIO5 (RX)
    // Modbus RTU typically uses 9600 baud, 8 data bits, 1 stop bit
    printf("Initializing UART1 (GPIO4, GPIO5) at 9600 baud...\n");
    auto uart = std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2);

    // Create Modbus client
    printf("Creating Modbus client...\n");
    auto modbus_client = std::make_shared<SafeModbusClient>(uart);

    // Create CO2 sensor
    printf("Creating CO2 sensor...\n");
    CO2Sensor sensor(modbus_client);

    printf("Setup complete!\n\n");

    // Continuous reading loop
    printf("Starting continuous CO2 reading...\n");
    printf("=====================================\n\n");

    int reading_count = 0;
    while (true)
    {
        // Read CO2 level
        float co2_ppm = sensor.getCO2Level();
        
        printf("Reading #%d: CO2 Level = %f ppm\n", ++reading_count, co2_ppm);

        // Wait 2 seconds before next reading
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main()
{
    stdio_init_all();

    // Wait for USB serial to stabilize
    sleep_ms(1000);

    printf("\n\nStarting CO2 Sensor Tests...\n\n");

    // Create test task
    xTaskCreate(
        test_co2_sensor_task,
        "CO2_Test",
        2048,
        nullptr,
        1,
        nullptr
    );

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    // Should never reach here
    while (true)
    {
        tight_loop_contents();
    }

    return 0;
}

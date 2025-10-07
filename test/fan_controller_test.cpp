//
// Created by Sheng Tai on 30/09/2025.
//

#include "CO2_Controller/FanController.h"
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

void test_fan_controller_task(void* params)
{
    printf("=== Fan Controller Test Starting ===\n\n");

    // Initialize UART1 on GPIO4 (TX) and GPIO5 (RX)
    printf("Initializing UART1 (GPIO4, GPIO5) at 9600 baud...\n");
    auto uart = std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2);

    // Create Modbus client
    printf("Creating Modbus client...\n");
    auto modbus_client = std::make_shared<SafeModbusClient>(uart);

    // Create fan controller
    printf("Creating fan controller...\n");
    FanController fan(modbus_client);
    int counter = 0;
    printf("Setup complete!\n\n");

    // Test 1: Set fan speed to 0 (stop)
    printf("Test 1: Setting fan speed to 0 (stop)\n");
    printf("--------------------------------------\n");
    fan.setSpeed(0);
    counter = fan.getCounter();
    printf("Fan speed set to: 0\n");
    printf("Current speed: %d\n\n", fan.getSpeed());
    printf("Current counter: %d\n\n", counter);
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Test 2: Set fan speed to 25% (250/1000)
    printf("Test 2: Setting fan speed to 25%% (250)\n");
    printf("----------------------------------------\n");
    fan.setSpeed(250);
    counter = fan.getCounter();
    printf("Fan speed set to: 250\n");
    printf("Current speed: %d\n\n", fan.getSpeed());
    printf("Current counter: %d\n\n", counter);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Test 3: Set fan speed to 50% (500/1000)
    printf("Test 3: Setting fan speed to 50%% (500)\n");
    printf("----------------------------------------\n");
    fan.setSpeed(500);
    counter = fan.getCounter();
    printf("Fan speed set to: 500\n");
    printf("Current speed: %d\n\n", fan.getSpeed());
    printf("Current counter: %d\n\n", counter);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Test 4: Set fan speed to 75% (750/1000)
    printf("Test 4: Setting fan speed to 75%% (750)\n");
    printf("----------------------------------------\n");
    fan.setSpeed(750);
    counter = fan.getCounter();
    printf("Fan speed set to: 750\n");
    printf("Current speed: %d\n\n", fan.getSpeed());
    printf("Current counter: %d\n\n", counter);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Test 5: Set fan speed to 100% (1000/1000)
    printf("Test 5: Setting fan speed to 100%% (1000)\n");
    printf("-----------------------------------------\n");
    fan.setSpeed(1000);
    counter = fan.getCounter();
    printf("Fan speed set to: 1000\n");
    printf("Current speed: %d\n\n", fan.getSpeed());
    printf("Current counter: %d\n\n", counter);
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Test 6: Ramp down gradually
    printf("Test 6: Ramping down fan speed\n");
    printf("-------------------------------\n");
    for (int speed = 1000; speed >= 0; speed -= 100)
    {
        printf("Setting speed to: %d\n", speed);
        fan.setSpeed(speed);
        counter = fan.getCounter();
        printf("Current counter: %d\n\n", counter);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf("Test 7: Counter test\n");
    fan.setSpeed(125);
    counter = fan.getCounter();
    printf("Current speed: %d\n", fan.getSpeed());
    printf("Current counter: %d\n", counter);
    vTaskDelay(pdMS_TO_TICKS(1000));
    counter = fan.getCounter();
    printf("Current counter: %d\n", counter);
    vTaskDelay(pdMS_TO_TICKS(2000));

    printf("\n");

    printf("=== Fan Controller Test Complete ===\n");

    // Keep task running
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main()
{
    stdio_init_all();

    // Wait for USB serial to stabilize
    sleep_ms(1000);

    printf("\n\nStarting Fan Controller Tests...\n\n");

    // Create test task
    xTaskCreate(
        test_fan_controller_task,
        "Fan_Test",
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

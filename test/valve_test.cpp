#include "FreeRTOS.h"
#include "task.h"
#include <cstdio>
#include <pico/stdio.h>
#include "hardware/timer.h"
#include "pico/time.h"
#include "CO2_Controller/CO2Valve.h"

extern "C" {
uint32_t read_runtime_ctr(void)
{
    return timer_hw->timerawl;
}
}

void test_co2_valve_task(void* params)
{
    printf("=== CO2 Valve Test Starting ===\n\n");

    // Initialize CO2 Valve
    printf("Initializing CO2 Valve...\n");
    CO2Valve valve;
    printf("CO2 Valve initialized successfully!\n\n");

    printf("Starting CO2 valve test...\n");

    int test_count = 0;
    while (true)
    {
        // Open valve
        printf("Test #%d: Opening CO2 valve...\n", ++test_count);
        valve.open();
        printf("CO2 valve is now OPEN.\n");
        // Wait 1 second before next test
        vTaskDelay(pdMS_TO_TICKS(1000));
        // Close valve
        printf("Test #%d: Closing CO2 valve...\n", test_count);
        valve.close();
        printf("CO2 valve is now CLOSED.\n");
        // Wait 10 seconds before next test
        vTaskDelay(pdMS_TO_TICKS(10 * 1000));
    }
}

int main()
{
    stdio_init_all();

    // Wait for USB serial to stabilize
    sleep_ms(1000);

    printf("\n\nStarting CO2 Valve Tests...\n\n");

    // Create test task
    xTaskCreate(
        test_co2_valve_task,
        "CO2_Valve_Test",
        1024,
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

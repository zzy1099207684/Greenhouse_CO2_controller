//
// Created by Sheng Tai on 04/10/2025.
//

#include "CO2_Controller/CO2Controller.h"

#include "PicoOsUart.h"
#include "FreeRTOS.h"
#include <cstdio>

#include "hardware/structs/timer.h"
#include "pico/stdio.h"
#include "pico/time.h"

extern "C" {
uint32_t read_runtime_ctr(void)
{
    return timer_hw->timerawl;
}
}

struct readInputTaskParams
{
    CO2Controller* co2Controller;
    PicoOsUart* uart;
};


void readInputTask(void* pvParameters)
{
    auto* params = static_cast<readInputTaskParams*>(pvParameters);
    auto* uart = params->uart;
    auto* co2_controller = params->co2Controller;
    char command_buffer[16];
    uint8_t command_buffer_index = 0;

    uint8_t buffer[1];
    printf("Enter new target CO2 level (ppm): \r\n");

    while (true)
    {
        if (uart->read(buffer, 1, portMAX_DELAY) > 0)
        {
            char ch = buffer[0];
            if (ch == '\r' || ch == '\n')
            {
                if (command_buffer_index > 0)
                {
                    command_buffer[command_buffer_index] = '\0'; // null terminate
                    uart->send("\r\n");
                    float new_target = strtof(command_buffer, nullptr);
                    co2_controller->setTargetCO2Level(new_target);
                    command_buffer[0] = '\0'; // clear buffer
                    command_buffer_index = 0;
                }
            }
            else if (ch == '\b' || ch == 127)
            {
                // process backspace
                if (command_buffer_index > 0)
                {
                    command_buffer_index--;
                    uart->send("\b \b"); // erase character
                }
            }
            else if (ch >= 32 && ch <= 126)
            {
                // process characters
                // leave one space for null terminator, \r or \n are not stored
                if (command_buffer_index < sizeof(command_buffer) - 1)
                {
                    command_buffer[command_buffer_index] = ch;
                    command_buffer_index++;
                    uart->write(buffer, 1); // echo back
                }
            }
        }
    }
}

int main()
{
    stdio_init_all();

    // Wait for USB serial to stabilize
    sleep_ms(1000);

    printf("\n\nStarting CO2 Controller Tests...\n\n");


    printf("Initializing UART1 (GPIO4, GPIO5) at 9600 baud...\n");
    auto uart = std::make_shared<PicoOsUart>(1, 4, 5, 9600, 2);

    // Create Modbus client
    printf("Creating Modbus client...\n");
    auto modbus_client = std::make_shared<SafeModbusClient>(uart);

    printf("Creating CO2 Controller...\n");
    CO2Controller co2_controller(modbus_client);
    co2_controller.setTargetCO2Level(500.0f); // Set target CO2 level to 500 ppm

    printf("Starting CO2 Controller...\n");
    co2_controller.start();


    printf("Creating tasks for reading user input...\n");
    static PicoOsUart uart_input(0, 0, 1, 115200);
    static readInputTaskParams read_input_task_params = {
        .co2Controller = &co2_controller,
        .uart = &uart_input
    };
    xTaskCreate(readInputTask, "ReadInputTask", 2048, &read_input_task_params, 1, nullptr);


    printf("Setup complete! Entering main loop...\n\n");
    vTaskStartScheduler();

    // Should never reach here
    while (true)
    {
        tight_loop_contents();
    }
}

//
// Created by Sheng Tai on 25/09/2025.
//

#include "Debug.h"
#include <cstdio>
#include <iostream>

QueueHandle_t Debug::debugQueue = nullptr;
bool Debug::initialized = false;

void Debug::init() {
    initialized = true;
    debugQueue = xQueueCreate(16, sizeof(debugEvent));
    xTaskCreate(printTask, "Debug", 512, nullptr, tskIDLE_PRIORITY + 1, nullptr);
}

void Debug::println(const char *format, uint32_t d1, uint32_t d2, uint32_t d3) {
    if (!initialized) {
        init();
    }
    debugEvent event{
        .timestamp = xTaskGetTickCount(), .format = format, .data = {d1, d2, d3}
    };
    xQueueSend(debugQueue, &event, 0);
}

void Debug::printTask(void *pvParameters) {
    char buffer[DEBUG_BUFFER_SIZE];
    debugEvent event{};
    while (true) {
        if (xQueueReceive(debugQueue, &event, portMAX_DELAY) == pdTRUE) {
            int len = snprintf(buffer, sizeof(buffer), event.format, event.data[0], event.data[1], event.data[2]);
            // trim if msg is too long
            if (len >= DEBUG_BUFFER_SIZE) {
                len = DEBUG_BUFFER_SIZE - 1;
                buffer[len] = '\0';
            }
            printf("[%lu] %s\n", event.timestamp, buffer);

            // std::cout << "[" << event.timestamp << "] " << buffer << std::endl;
        }
    }
}
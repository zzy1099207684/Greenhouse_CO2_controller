//
// Created by Sheng Tai on 25/09/2025.
//

#ifndef LAB4_1_DEBUG_H
#define LAB4_1_DEBUG_H
#include <FreeRTOS.h>
#include <queue.h>

struct debugEvent {
    TickType_t timestamp;
    const char *format;
    uint32_t data[3];
};

class Debug {
  public:
    static void init();
    static void println(const char *format, uint32_t d1, uint32_t d2, uint32_t d3);

  private:
    static constexpr int DEBUG_BUFFER_SIZE = 256;
    static void printTask(void *pvParameters);
    static QueueHandle_t debugQueue;
    static bool initialized;
};

#endif // LAB4_1_DEBUG_H

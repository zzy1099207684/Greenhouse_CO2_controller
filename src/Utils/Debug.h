#ifndef DEBUG_H
#define DEBUG_H

#ifdef ENABLE_DEBUG_PRINT
#include "FreeRTOS.h"
#include "queue.h"
#include <cstdio>

class Debug
{
public:
    Debug() = delete;
    static void init();
    static void println(const char* fmt, ...);

private:
    static constexpr size_t MSG_SIZE = 128;
    static constexpr size_t QUEUE_LENGTH = 5;
    static QueueHandle_t log_queue;
    static void logTask(void* param);
    struct LogMsg
    {
        uint32_t timestamp;
        char msg[MSG_SIZE];
    };
};

#else
class Debug
{
public:
    static void init()
    {
    }

    static void println(const char*, ...)
    {
    }
};

#endif

#endif // DEBUG_H

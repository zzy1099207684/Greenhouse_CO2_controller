#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_ENABLE
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
    static constexpr size_t QUEUE_LENGTH = 10;
    static QueueHandle_t log_queue;
    static void logTask(void* param);
    struct LogMsg
    {
        uint32_t timestamp;
        char msg[MSG_SIZE];
    };
};

#define DPRINT(...) Debug::print(__VA_ARGS__)

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

#define DPRINT(...) ((void)0)
#endif

#endif // DEBUG_H

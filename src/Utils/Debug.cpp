#include "Debug.h"

#ifdef DEBUG_ENABLE

#include <cstdarg>
QueueHandle_t Debug::log_queue = nullptr;

void Debug::init()
{
    log_queue = xQueueCreate(QUEUE_LENGTH, sizeof(LogMsg));

    if (log_queue == nullptr)
    {
        printf("Error: Debug Queue Create Failed\n");
        return;
    }

    BaseType_t ret = xTaskCreate(logTask, "LogTask", 1024, nullptr, 1, nullptr);
    if (ret != pdPASS)
    {
        printf("Error: LogTask Create Failed\n");
    }
}

void Debug::println(const char* fmt, ...)
{
    LogMsg log{};
    log.timestamp = xTaskGetTickCount();

    char temp[MSG_SIZE];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(temp, sizeof(temp), fmt, args);
    va_end(args);

    snprintf(log.msg, sizeof(log.msg), "%s", temp);

    if (xQueueSend(log_queue, &log, 0) != pdTRUE)
    {
        printf("[%lu] [WARN] Log queue full, message dropped\n", xTaskGetTickCount());
    }

    if (len >= static_cast<int>(MSG_SIZE))
    {
        LogMsg warn{};
        warn.timestamp = log.timestamp;
        snprintf(warn.msg, sizeof(warn.msg), "[WARN] Previous log too long, truncated!");

        if (xQueueSend(log_queue, &warn, 0) != pdTRUE)
        {
            printf("[%lu] [WARN] Log queue full, message dropped\n", xTaskGetTickCount());
        }
    }
}

void Debug::logTask(void* param)
{
    (void)param;
    LogMsg log{};

    while (true)
    {
        if (xQueueReceive(log_queue, &log, portMAX_DELAY) == pdTRUE)
        {
            printf("[%lu] %s\n", log.timestamp, log.msg);
        }
    }
}

#endif

//
// Created by Sheng Tai on 30/09/2025.
//

#include "FanController.h"
#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "event_groups.h"
#include "Utils/Debug.h"
#include "event_bits_def.h"

FanController::FanController(const std::shared_ptr<SafeModbusClient>& safe_modbus_client,
                             EventGroupHandle_t event_group)
    : safe_modbus_client(safe_modbus_client),
      speed_register(safe_modbus_client, SERVER_ADDR, SPEED_REGISTER_ADDR, true),
      counter_register(safe_modbus_client, SERVER_ADDR, COUNTER_REGISTER_ADDR, false),
      event_group(event_group),
      fan_check_timer(nullptr),
      speed(0),
      zero_count(0)
{
    // create periodic timer for fan health check, initially not started
    fan_check_timer = xTimerCreate("FanCheckTimer", pdMS_TO_TICKS(FAN_CHECK_INTERVAL_MS),
                                   pdTRUE, this, fanCheckTimerCallback);
}

void FanController::setSpeed(int new_speed)
{
    if (new_speed < 0)
    {
        new_speed = 0;
    }
    if (new_speed > MAX_SPEED)
    {
        new_speed = MAX_SPEED;
    }
    speed_register.write(new_speed);
    speed = new_speed;

    // start or stop fan health check based on speed
    if (new_speed >= MIN_WORKING_SPEED)
    {
        // fan is running above min working speed, start health check if not already running
        if (xTimerIsTimerActive(fan_check_timer) == pdFALSE)
        {
            xTimerStart(fan_check_timer, 0);
        }
    }
    else
    {
        // fan is stopped or too slow to work properly, stop health check
        xTimerStop(fan_check_timer, 0);
        zero_count = 0;
    }
}

int FanController::getSpeed() const
{
    return speed;
}


/**
 * Periodic callback to check fan health by reading the counter register
 * Counter auto-resets to 0 after read, so if fan is working, it should not be 0 consecutively
 */
void FanController::fanCheckTimerCallback(TimerHandle_t xTimer)
{
    auto* self = static_cast<FanController*>(pvTimerGetTimerID(xTimer));
    int counter = self->counter_register.read();
    // int counter = 0; // for testing purpose, set counter = 0 always
    if (counter == 0)
    {
        // counter is 0, increment zero count
        self->zero_count++;
        Debug::println("Fan counter is 0, zero_count=%d", self->zero_count);
        if (self->zero_count >= FAN_FAILURE_THRESHOLD)
        {
            // fan is broken, set warning
            xEventGroupSetBits(self->event_group, FAN_WARNING);
        }
    }
    else
    {
        // counter is not 0, fan is working normally, reset zero count and clear warning
        Debug::println("Fan working normally, counter=%d", counter);
        xEventGroupClearBits(self->event_group, FAN_WARNING);
        self->zero_count = 0;
    }
}

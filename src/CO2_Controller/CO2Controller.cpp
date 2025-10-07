//
// Created by Sheng Tai on 30/09/2025.
//

#include "CO2Controller.h"
#include <memory>
#include "Modbus/SafeModbusClient.h"
#include "Modbus/SafeModbusRegister.h"
#include "FreeRTOS.h"
#include "task.h"
#include "CO2Valve.h"
#include "CO2Sensor.h"
#include "FanController.h"
#include <cstdio>
#include "Utils/Debug.h"

#define CO2_WARNING (1<<7) //warning from co2 controller

CO2Controller::CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client,
                             EventGroupHandle_t event_group)
    : sensor(safe_modbus_client),
      fan(safe_modbus_client),
      event_group(event_group),
      closeValveTimerHandle(nullptr),
      mixingTimerHandle(nullptr),
      CO2_value(0.0f),
      CO2_setpoint(800.0f),
      controller_state(IDLE)
{
    xTaskCreate(controlTask, "CO2ControlTask", TASK_STACK_SIZE / sizeof(StackType_t), this,
                tskIDLE_PRIORITY + TASK_PRIORITY, nullptr);
    closeValveTimerHandle = xTimerCreate("CloseValveTimer", pdMS_TO_TICKS(VALVE_OPEN_TIME_MIN),
                                         pdFALSE, this, closeValveTimerCallback);
    mixingTimerHandle = xTimerCreate("MixingTimer", pdMS_TO_TICKS(MIXING_TIME),
                                     pdFALSE, this, mixingTimerCallback);
}

/**
 *
 * @param ppm Target CO2 level in ppm (200 - 1500)
 * @return true if set successfully, false if invalid value
 */
bool CO2Controller::setCO2Setpoint(float ppm)
{
    if (ppm < CO2_SETPOINT_MIN || ppm > CO2_SETPOINT_MAX)
    {
        Debug::println("Invalid CO2 setpoint: %.2f ppm. Must be between %d and %d ppm.",
                       ppm, CO2_SETPOINT_MIN, CO2_SETPOINT_MAX);
        return false;
    }
    Debug::println("Set CO2 setpoint to %.2f ppm", ppm);
    CO2_setpoint = ppm;
    return true;
}

/**
 *
 * @return Target CO2 level in ppm, set by user. (float)
 */
float CO2Controller::getCO2Setpoint() const
{
    return CO2_setpoint;
}

/**
 *
 * @return Current CO2 level in ppm, read from sensor. (float)
 */
float CO2Controller::getCO2Value() const
{
    return CO2_value; // cached value, updated in control loop
}

/**
 *
 * @return Current fan speed as percentage (0.0 - 100.0)
 */
float CO2Controller::getFanSpeed() const
{
    return static_cast<float>(fan.getSpeed()) / FAN_SPEED_MAX * 100.0f; // return as percentage
}


/**
 * Callback to close the valve after timer expires
 */
void CO2Controller::closeValveTimerCallback(TimerHandle_t xTimer)
{
    auto* self = static_cast<CO2Controller*>(pvTimerGetTimerID(xTimer));
    self->valve.close();
    self->controller_state = INJECTION_MIXING;
    xTimerStart(self->mixingTimerHandle, 0);
    Debug::println("Valve closed, starting mixing timer.");
}

/**
 * Callback to end mixing state after timer expires
 */
void CO2Controller::mixingTimerCallback(TimerHandle_t xTimer)
{
    auto* self = static_cast<CO2Controller*>(pvTimerGetTimerID(xTimer));
    self->controller_state = INJECTION_READY;
    Debug::println("Mixing time ended, ready for next injection check.");
}

void CO2Controller::controlTask(void* pvParameters)
{
    auto* self = static_cast<CO2Controller*>(pvParameters);
    auto& sensor = self->sensor;
    auto& fan = self->fan;
    auto& valve = self->valve;
    auto& CO2_value = self->CO2_value;
    auto& CO2_setpoint = self->CO2_setpoint;
    auto& controller_state = self->controller_state;
    auto& closeValveTimerHandle = self->closeValveTimerHandle;
    auto& mixingTimerHandle = self->mixingTimerHandle;

    while (true)
    {
        // control loop every 1 second
        vTaskDelay(pdMS_TO_TICKS(CONTROL_INTERVAL_MS));
        CO2_value = sensor.getCO2Level(); // update CO2 value

        // check for emergency first
        if (CO2_value > CO2_CRITICAL && controller_state != EMERGENCY)
        {
            valve.close();
            fan.setSpeed(FAN_SPEED_MAX);
            xTimerStop(closeValveTimerHandle, 0);
            xTimerStop(mixingTimerHandle, 0);
            controller_state = EMERGENCY;
            xEventGroupSetBits(self->event_group, CO2_WARNING);
            Debug::println("CO2 level critical: %.2f ppm! Entering EMERGENCY state.", CO2_value);
        }

        switch (controller_state)
        {
        case IDLE:
            // CO2 value is below the setpoint
            if (CO2_value < CO2_setpoint - DEADBAND)
            {
                controller_state = INJECTION_READY;
                Debug::println("CO2 level low: %.2f ppm. Preparing for injection.", CO2_value);
            }
            // CO2 value is above the setpoint
            else if (CO2_value > CO2_setpoint + DEADBAND)
            {
                controller_state = VENTILATING;
                Debug::println("CO2 level high: %.2f ppm. Starting ventilation.", CO2_value);
            }
            break;
        case VENTILATING:
            if (CO2_value <= CO2_setpoint)
            {
                fan.setSpeed(0);
                controller_state = IDLE;
                Debug::println("CO2 level normal: %.2f ppm. Stopping ventilation.", CO2_value);
            }
            else
            {
                float diff = CO2_value - CO2_setpoint;
                int speed = static_cast<int>(diff * FAN_SPEED_K);
                if (speed > FAN_SPEED_MAX)
                {
                    speed = FAN_SPEED_MAX;
                }
                else if (speed < FAN_SPEED_MIN)
                {
                    speed = FAN_SPEED_MIN;
                }
                fan.setSpeed(speed);
            }

            break;
        case INJECTION_READY:
            if (CO2_value >= CO2_setpoint)
            {
                controller_state = IDLE;
                Debug::println("CO2 level normal: %.2f ppm. Injection not needed.", CO2_value);
            }
            else
            {
                float diff = CO2_setpoint - CO2_value;
                // calculate valve open time, in ms
                int open_time = static_cast<int>(diff * VALVE_OPEN_TIME_K);
                if (open_time > VALVE_OPEN_TIME_MAX)
                {
                    open_time = VALVE_OPEN_TIME_MAX;
                }
                else if (open_time < VALVE_OPEN_TIME_MIN)
                {
                    open_time = VALVE_OPEN_TIME_MIN;
                }
                xTimerChangePeriod(closeValveTimerHandle, pdMS_TO_TICKS(open_time), 0);
                xTimerStart(closeValveTimerHandle, 0);
                valve.open();
                controller_state = INJECTION_FILLING;
                Debug::println("Injecting CO2 for %d ms.", open_time);
            }
            break;
        case INJECTION_FILLING:
            // do nothing, wait for timer to close valve
            break;
        case INJECTION_MIXING:
            // do nothing, wait for timer to end mixing
            break;
        case EMERGENCY:
            if (CO2_value < CO2_SETPOINT_MAX)
            {
                // below max setpoint, exit emergency, let normal control task handle the rest (fan will be adjusted in next loop)
                controller_state = IDLE;
                Debug::println("CO2 level safe: %.2f ppm. Exiting EMERGENCY state.", CO2_value);
                xEventGroupClearBits(self->event_group, CO2_WARNING);
            }
            break;
        }
    }
}

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
#include "event_bits_def.h"

CO2Controller::CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client,
                             EventGroupHandle_t event_group)
    : sensor(safe_modbus_client),
      fan(safe_modbus_client, event_group),
      event_group(event_group),
      closeValveTimerHandle(nullptr),
      mixingTimerHandle(nullptr),
      CO2_value(0.0f),
      CO2_setpoint(0.0f),
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
    if (self->controller_state == INJECTING)
    {
        self->valve.close();
        self->controller_state = MIXING;
        xTimerStart(self->mixingTimerHandle, 0);
        Debug::println("Valve closed, starting mixing timer.");
    }
}

/**
 * Callback to end mixing state after timer expires
 */
void CO2Controller::mixingTimerCallback(TimerHandle_t xTimer)
{
    auto* self = static_cast<CO2Controller*>(pvTimerGetTimerID(xTimer));
    if (self->controller_state == MIXING)
    {
        self->controller_state = IDLE;
        Debug::println("Mixing time ended, Back to IDLE and ready for next check.");
    }
}

// this version will only ventilate at emergency situation
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
                fan.setSpeed(0); // ensure fan is off
                Debug::println("CO2 level %.2f < setpoint-deadband %.2f ppm. Preparing for injection.",
                               CO2_value, CO2_setpoint - DEADBAND);
                float diff = CO2_setpoint - CO2_value + DEADBAND; // inject to upper bound since it drops naturally
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
                controller_state = INJECTING;
                Debug::println("Injecting CO2 for %d ms.", open_time);
            }
            // CO2 value is above the setpoint + ventilation start threshold. i.e. the CO2 is too high
            else if (CO2_value > CO2_setpoint + VENTILATION_THRESHOLD)
            {
#ifdef WITH_ACTIVE_VENTILATION
                controller_state = VENTILATING;
                valve.close();
                Debug::println("CO2 level %.2f >> setpoint %.2f ppm. Too high, starting ventilation.",
                               CO2_value, CO2_setpoint);
#else
                valve.close();
                Debug::println("CO2 level %.2f >> setpoint %.2f ppm.", CO2_value, CO2_setpoint);
                Debug::println("But who cares, it will drop anyway ( (╯°□°)╯︵ ┻━┻.");
#endif
            }
            else
            {
                // within deadband, do nothing
                // need to ensure everything is off and normal
                // in case someone cut the power during ventilation or injection and it will stay on
                valve.close();
                fan.setSpeed(0);
                xTimerStop(closeValveTimerHandle, 0);
                xTimerStop(mixingTimerHandle, 0);
                xEventGroupClearBits(self->event_group, CO2_WARNING);
                Debug::println("CO2 level normal: %.2f ppm. Everything is good.", CO2_value);
            }
            break;
        case VENTILATING:
#ifdef WITH_ACTIVE_VENTILATION
            // when the co2 level is much higher than the setpoint
            // i.e. higher than setpoint + ventilation threshold
            // start ventilation until CO2 is back to the setpoint (with deadband)
            if (CO2_value <= CO2_setpoint + DEADBAND) // stops at upper bound since it drops naturally
            {
                fan.setSpeed(0);
                controller_state = IDLE;
                Debug::println("CO2 level normal: %.2f ppm. Stopping ventilation.", CO2_value);
            }
            else
            {
                float diff = CO2_value - (CO2_setpoint + DEADBAND);
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
#else
            // This version does not have active ventilation
            // Only emergency state will turn on the fan
#endif
            break;
        case INJECTING:
            // do nothing, wait for timer to close valve
            break;
        case MIXING:
            // check for emergency first
            // do nothing, wait for timer to end mixing
            // BTW (emergency check is already done before switch statement)

            break;
        case EMERGENCY:
            // start ventilation until CO2 is back to the setpoint (with deadband)
            if (CO2_value <= CO2_setpoint + DEADBAND) // back to normal range + deadband since it drops naturally
            {
                fan.setSpeed(0);
                controller_state = IDLE;
                Debug::println("CO2 level normal: %.2f ppm. Exiting EMERGENCY state.", CO2_value);
                xEventGroupClearBits(self->event_group, CO2_WARNING);
            }
            else
            {
                float diff = CO2_value - (CO2_setpoint + DEADBAND);
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
        }
    }
}

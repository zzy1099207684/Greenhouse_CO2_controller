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

CO2Controller::CO2Controller(const std::shared_ptr<SafeModbusClient>& safe_modbus_client)
    : valve(),
      sensor(safe_modbus_client),
      fan(safe_modbus_client),
      target_co2_level(800.0f),
      emergency_state(false)

{
}

/**
 *
 * @param ppm Target CO2 level in ppm (0 - 1500)
 * @return true if set successfully, false if invalid value
 */
bool CO2Controller::setTargetCO2Level(float ppm)
{
    if (ppm < 0.0f || ppm > CO2_TARGET_MAX)
    {
        return false; // invalid target level
    }
    target_co2_level = ppm;
    return true;
}

/**
 *
 * @return Target CO2 level in ppm, set by user. (float)
 */
float CO2Controller::getTargetCO2Level() const
{
    return target_co2_level;
}

/**
 *
 * @return Current CO2 level in ppm, read from sensor. (float)
 */
float CO2Controller::getCurrentCO2Level()
{
    return sensor.getCO2Level();
}

/**
 *
 * @return Current fan speed as percentage (0.0 - 100.0)
 */
float CO2Controller::getFanSpeed() const
{
    return static_cast<float>(fan.getSpeed()) / MAX_FAN_SPEED * 100.0f; // return as percentage
}

void CO2Controller::start()
{
    xTaskCreate(controlTask, "CO2ControlTask", CONTROL_TASK_STACK_SIZE / sizeof(StackType_t), this,
                tskIDLE_PRIORITY + CONTROL_TASK_PRIORITY, nullptr);
    xTaskCreate(emergencyMonitorTask, "CO2EmergencyTask", EMERGENCY_TASK_STACK_SIZE / sizeof(StackType_t), this,
                tskIDLE_PRIORITY + EMERGENCY_TASK_PRIORITY, nullptr);
}


void CO2Controller::controlTask(void* pvParameters)
{
    auto* self = static_cast<CO2Controller*>(pvParameters);
    while (true)
    {
        // normal control loop every 500ms
        vTaskDelay(pdMS_TO_TICKS(500));

        if (self->emergency_state)
        {
            // In emergency state, do nothing
            Debug::println("In emergency state, waiting...", 0, 0, 0);
            continue;
        }

        // CO2 level low
        if (self->sensor.getCO2Level() < self->target_co2_level - DEADBAND)
        {
            while (self->sensor.getCO2Level() < self->target_co2_level)
            {
                // CO2 level is below target, open valve
                float diff = self->target_co2_level - self->sensor.getCO2Level();
                int open_time = static_cast<int>(diff * VALVE_TIME_K + VALVE_TIME_B); // in ms
                if (open_time > VALVE_MAX_OPEN_TIME)
                {
                    open_time = VALVE_MAX_OPEN_TIME;
                }
                // ensure not in emergency state before action
                if (self->emergency_state)
                {
                    break; // exit the loop
                }
                Debug::println("Opening valve for %d ms, CO2 level: %d ppm, target: %d ppm (decimal truncated)",
                               open_time,
                               static_cast<int>(self->sensor.getCO2Level()), static_cast<int>(self->target_co2_level));
                self->fan.setSpeed(0);
                self->valve.open();
                vTaskDelay(pdMS_TO_TICKS(open_time));
                // ensure not in emergency state before action
                if (self->emergency_state)
                {
                    break; // exit the loop
                }
                self->valve.close();
                // wait for some time to allow CO2 to mix
                Debug::println("Waiting for %d ms to allow CO2 to mix", VALVE_WAIT_TIME, 0, 0);
                vTaskDelay(pdMS_TO_TICKS(VALVE_WAIT_TIME));
            }
        }

        // CO2 level high
        if (self->sensor.getCO2Level() > self->target_co2_level + DEADBAND)
        {
            while (self->sensor.getCO2Level() > self->target_co2_level)
            {
                // CO2 level is above target, turn on fan
                float diff = self->sensor.getCO2Level() - self->target_co2_level;

                int speed = static_cast<int>((diff * FAN_SPEED_K + FAN_SPEED_B) / 100 * MAX_FAN_SPEED);
                // scale to 0-1000
                if (speed > MAX_FAN_SPEED)
                {
                    speed = MAX_FAN_SPEED;
                }
                // ensure not in emergency state before action
                if (self->emergency_state)
                {
                    break; // exit the loop
                }
                self->fan.setSpeed(speed);
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            self->fan.setSpeed(0); // turn off fan when back to target
        }
    }
}

void CO2Controller::emergencyMonitorTask(void* pvParameters)
{
    auto* self = static_cast<CO2Controller*>(pvParameters);
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(EMERGENCY_CHECK_INTERVAL_MS));
        if (self->sensor.getCO2Level() >= CO2_CRITICAL)
        {
            self->emergency_state = true; // enter emergency state
            // close valve and turn on fan at max speed
            self->valve.close();
            self->fan.setSpeed(MAX_FAN_SPEED);
            Debug::println("EMERGENCY: CO2 level critical at %d ppm! Turning on fan at full speed...",
                           static_cast<int>(self->sensor.getCO2Level()), 0, 0);
            // run until CO2 drops to target level
            while (self->sensor.getCO2Level() > self->target_co2_level)
            {
                vTaskDelay(pdMS_TO_TICKS(10)); // short delay to avoid busy loop
            }
            // exit emergency state
            Debug::println("CO2 level back to safe level at %d ppm. Exiting emergency state.",
                           static_cast<int>(self->sensor.getCO2Level()), 0, 0);
            self->fan.setSpeed(0); // turn off fan
            self->emergency_state = false;
        }
    }
}

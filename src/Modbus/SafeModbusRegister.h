#ifndef SAFEMODBUSREGISTER_H
#define SAFEMODBUSREGISTER_H

#include "ModbusRegister.h"
#include "SafeModbusClient.h"
#include <memory>

class SafeModbusRegister
{
public:
    SafeModbusRegister(const std::shared_ptr<SafeModbusClient>& safe_client,
                       int server_address,
                       int register_address,
                       bool holding_register = true);

    uint16_t read();
    void write(uint16_t value);

private:
    ModbusRegister modbusRegister;
    std::shared_ptr<Fmutex> mutex;
};

#endif //SAFEMODBUSREGISTER_H

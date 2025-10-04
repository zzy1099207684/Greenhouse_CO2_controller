#include "SafeModbusClient.h"

SafeModbusClient::SafeModbusClient(std::shared_ptr<PicoOsUart> uart)
    : ModbusClient(uart),
      mutex(std::make_shared<Fmutex>())
{
}

std::shared_ptr<Fmutex> SafeModbusClient::getMutex() const
{
    return mutex;
}

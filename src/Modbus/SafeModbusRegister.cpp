#include "SafeModbusRegister.h"
#include <mutex>

SafeModbusRegister::SafeModbusRegister(const std::shared_ptr<SafeModbusClient>& safe_client,
                                       const int server_address,
                                       const int register_address,
                                       const bool holding_register)
    : modbusRegister(safe_client, server_address, register_address, holding_register),
      mutex(safe_client->getMutex()) {
}

uint16_t SafeModbusRegister::read() {
    std::lock_guard<Fmutex> lock(*mutex);
    return modbusRegister.read();
}

void SafeModbusRegister::write(uint16_t value) {
    std::lock_guard<Fmutex> lock(*mutex);
    modbusRegister.write(value);
}

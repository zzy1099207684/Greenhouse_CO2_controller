#ifndef SAFEMODBUSCLIENT_H
#define SAFEMODBUSCLIENT_H

#include "ModbusClient.h"
#include <memory>

class SafeModbusClient : public ModbusClient {
public:
    explicit SafeModbusClient(std::shared_ptr<PicoOsUart> uart);

    [[nodiscard]] std::shared_ptr<Fmutex> getMutex() const;

private:
    std::shared_ptr<Fmutex> mutex;
};

#endif //SAFEMODBUSCLIENT_H


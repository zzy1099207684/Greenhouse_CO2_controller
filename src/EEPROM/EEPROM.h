//
// Created by zbinc on 21.9.2025.
//

#ifndef EEPROMMANAGER_H
#define EEPROMMANAGER_H

#include "../../rp2040-freertos/src/i2c/PicoI2C.h"

class EEPROM {
private:
    PicoI2C& i2c;
    uint8_t deviceAddr;

    static constexpr uint8_t PAGE_SIZE = 64;
    static constexpr uint32_t EEPROM_WRITE_TIMEOUT = 1000;

    static constexpr uint16_t ADDR_CO2 = 0x00;
    static constexpr uint16_t ADDR_SSID = 0x10;
    static constexpr uint16_t ADDR_PWD = 0x40;

    static constexpr uint8_t MAX_ARRAY_LENGTH = 32;

    //bool isBusy();
    bool writeBytes(uint16_t address, const uint8_t* data, uint16_t length);
    bool readBytes(uint16_t address, uint8_t* data, uint16_t length) const;

    bool writeCharArray(const char* arr, uint16_t addr);


    bool readCharArray(char* arr, uint16_t addr) const;


public:
    EEPROM(PicoI2C& i2cInterface, uint8_t deviceAddr);
    bool writeCO2Value(uint16_t co2_value);
    bool readCO2Value(uint16_t& co2_value) const;
    bool writeSSID(const char* ssid);
    bool writePWD(const char* pwd);
    bool readSSID(char *ssid) const;
    bool readPWD(char *pwd) const;
    // bool writeNetworkParams();
    // bool readNetworkParams();

};



#endif //EEPROMMANAGER_H

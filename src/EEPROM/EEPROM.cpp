//
// Created by zbinc on 21.9.2025.
//

#include "EEPROM.h"
#include <cstring>



EEPROM::EEPROM(const std::shared_ptr<PicoI2C> &i2c_bus) : i2c(i2c_bus){
}


bool EEPROM::writeBytes(const uint16_t address, const uint8_t *data, const uint16_t length) {
    uint16_t bytes_written = 0;

    while (bytes_written < length) {
        const uint16_t current_addr = address + bytes_written;
        const uint16_t bytes_until_page_end = PAGE_SIZE - (current_addr % PAGE_SIZE);
        const uint16_t bytes_to_write = (length - bytes_written < bytes_until_page_end) ?
                                  (length - bytes_written) : bytes_until_page_end;
        // Prepare write buffer
        uint8_t write_buffer[PAGE_SIZE + 2];
        write_buffer[0] = (current_addr >> 8) & 0xFF;
        write_buffer[1] = current_addr & 0xFF;

        // Copy data into write buffer
        for (uint16_t i = 0; i < bytes_to_write; i++) {
            write_buffer[2 + i] = data[bytes_written + i];
        }

        if (i2c->write(deviceAddr, write_buffer, bytes_to_write + 2) != static_cast<uint>(bytes_to_write + 2)) {
            return false;
        }
        // if (isBusy()) {
        //     return false;
        // }
        bytes_written += bytes_to_write; //in case the page is full
    }
    return true;
}

bool EEPROM::readBytes(const uint16_t address, uint8_t *data, const uint16_t length) const {
    //Prepare address buffer for the read operation
    uint8_t addr_buffer[2];
    addr_buffer[0] = (address >> 8) & 0xFF;
    addr_buffer[1] = address & 0xFF;
    return i2c->transaction(deviceAddr, addr_buffer, 2, data, length) == static_cast<uint>(2 + length);
}

bool EEPROM::writeCO2Value(const int co2_value) {
    uint8_t data[2];
    data[0] = (co2_value >> 8) & 0xFF;
    data[1] = co2_value & 0xFF;
    return writeBytes(ADDR_CO2, data, 2);
}

bool EEPROM::readCO2Value(int &co2_value) const {
    uint8_t data[2];
    if (!readBytes(ADDR_CO2, data, 2)) return false;
    co2_value = (data[0] << 8) | data[1];
    return true;
}

bool EEPROM::writeCharArray(const char *arr, const uint16_t addr) {
    uint8_t length = strlen(arr);
    uint8_t data[MAX_ARRAY_LENGTH + 1];
    data[0] = length;
    memcpy(data + 1, arr, length);
    return writeBytes(addr, data, length + 1);
}

bool EEPROM::writeSSID(const char *ssid) {
    return writeCharArray(ssid, ADDR_SSID);
}

bool EEPROM::writePWD(const char *pwd) {
    return writeCharArray(pwd, ADDR_PWD);
}

bool EEPROM::readCharArray(char *arr, const uint16_t addr) const {
    uint8_t data[MAX_ARRAY_LENGTH];
    if (!readBytes(addr, data, MAX_ARRAY_LENGTH)) return false;
    uint8_t length = data[0];
    for (uint8_t i = 0; i < length; i++) {
        arr[i] = data[i + 1];
    }
    arr[length] = '\0';
    return true;
}

bool EEPROM::readSSID(char *ssid) const {
    return readCharArray(ssid, ADDR_SSID);
}

bool EEPROM::readPWD(char *pwd) const {
    return readCharArray(pwd, ADDR_PWD);
}





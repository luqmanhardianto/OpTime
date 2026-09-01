#pragma once

#include <stdint.h>

#include <Wire.h>

#include "common/Status.h"

class I2cHal
{
public:
    StatusCode begin();

    StatusCode write(uint8_t deviceAddress, const uint8_t* data, uint8_t length);
    StatusCode read(uint8_t deviceAddress, uint8_t* data, uint8_t length);

    StatusCode writeRegister(uint8_t deviceAddress, uint8_t reg, uint8_t value);
    StatusCode readRegister(uint8_t deviceAddress, uint8_t reg, uint8_t& value);

private:
    static constexpr uint16_t I2C_TIMEOUT_MS = 100U;
};

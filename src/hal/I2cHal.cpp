#include "hal/I2cHal.h"

#include <Arduino.h>

namespace
{
constexpr uint32_t I2C_CLOCK_HZ = 400000U;
constexpr uint8_t DS3231_ADDRESS = 0x68U;

bool waitForWireReady(uint16_t timeoutMs)
{
    uint32_t start = millis();
    while (Wire.available() == 0 && (millis() - start) < timeoutMs)
    {
    }
    return true;
}
}  // namespace

StatusCode I2cHal::begin()
{
    Wire.begin();
    Wire.setClock(I2C_CLOCK_HZ);
    return StatusCode::OK;
}

StatusCode I2cHal::write(uint8_t deviceAddress, const uint8_t* data, uint8_t length)
{
    if (data == nullptr || length == 0U)
    {
        return StatusCode::INVALID_PARAMETER;
    }

    Wire.beginTransmission(deviceAddress);
    for (uint8_t i = 0; i < length; ++i)
    {
        Wire.write(data[i]);
    }

    const uint8_t status = Wire.endTransmission();
    if (status == 0U)
    {
        return StatusCode::OK;
    }

    return StatusCode::ERROR;
}

StatusCode I2cHal::read(uint8_t deviceAddress, uint8_t* data, uint8_t length)
{
    if (data == nullptr || length == 0U)
    {
        return StatusCode::INVALID_PARAMETER;
    }

    const uint8_t bytesRead = Wire.requestFrom(deviceAddress, length);
    if (bytesRead != length)
    {
        return StatusCode::ERROR;
    }

    for (uint8_t i = 0; i < length; ++i)
    {
        data[i] = static_cast<uint8_t>(Wire.read());
    }

    return StatusCode::OK;
}

StatusCode I2cHal::writeRegister(uint8_t deviceAddress, uint8_t reg, uint8_t value)
{
    const uint8_t payload[2] = {reg, value};
    return write(deviceAddress, payload, 2U);
}

StatusCode I2cHal::readRegister(uint8_t deviceAddress, uint8_t reg, uint8_t& value)
{
    StatusCode status = write(deviceAddress, &reg, 1U);
    if (status != StatusCode::OK)
    {
        return status;
    }

    return read(deviceAddress, &value, 1U);
}

#include "drivers/RtcDriver.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t kTimeRegisterCount = 7U;
}

RtcDriver::RtcDriver() : lastStatus_(StatusCode::NOT_READY), initialized_(false), oscillatorStopped_(false) {}

StatusCode RtcDriver::begin()
{
    initialized_ = false;
    oscillatorStopped_ = false;

    if (i2cHal_.begin() != StatusCode::OK)
    {
        lastStatus_ = StatusCode::ERROR;
        return lastStatus_;
    }

    uint8_t statusValue = 0U;
    StatusCode status = readStatusRegister(statusValue);
    if (status != StatusCode::OK)
    {
        lastStatus_ = status;
        return lastStatus_;
    }

    oscillatorStopped_ = ((statusValue & kOscillatorStopFlag) != 0U);
    initialized_ = true;
    lastStatus_ = oscillatorStopped_ ? StatusCode::NOT_READY : StatusCode::OK;
    return lastStatus_;
}

StatusCode RtcDriver::read(DateTime& time)
{
    if (!initialized_)
    {
        return StatusCode::NOT_READY;
    }

    uint8_t raw[7] = {0U};
    const uint8_t registerAddress = kRegisterSeconds;
    StatusCode status = i2cHal_.write(kRtcAddress, &registerAddress, 1U);
    if (status != StatusCode::OK)
    {
        lastStatus_ = status;
        return lastStatus_;
    }

    status = i2cHal_.read(kRtcAddress, raw, kTimeRegisterCount);
    if (status != StatusCode::OK)
    {
        lastStatus_ = status;
        return lastStatus_;
    }

    time.second = bcdToDecimal(raw[0]);
    time.minute = bcdToDecimal(raw[1]);
    time.hour = bcdToDecimal(raw[2]);
    time.day = bcdToDecimal(raw[3]);
    time.date = bcdToDecimal(raw[4]);
    time.month = bcdToDecimal(raw[5]);
    time.year = static_cast<uint16_t>(bcdToDecimal(raw[6]) + 2000U);

    lastStatus_ = StatusCode::OK;
    return lastStatus_;
}

StatusCode RtcDriver::set(const DateTime& time)
{
    if (!initialized_ || !validateTime(time))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    uint8_t payload[8] = {kRegisterSeconds,
                          decimalToBcd(time.second),
                          decimalToBcd(time.minute),
                          decimalToBcd(time.hour),
                          decimalToBcd(time.day),
                          decimalToBcd(time.date),
                          decimalToBcd(time.month),
                          decimalToBcd(static_cast<uint8_t>(time.year % 100U))};

    StatusCode status = i2cHal_.write(kRtcAddress, payload, sizeof(payload));
    if (status != StatusCode::OK)
    {
        lastStatus_ = status;
        return lastStatus_;
    }

    oscillatorStopped_ = false;
    lastStatus_ = StatusCode::OK;
    return lastStatus_;
}

bool RtcDriver::isValid()
{
    if (!initialized_)
    {
        return false;
    }

    return !oscillatorStopped_ && (lastStatus_ == StatusCode::OK);
}

uint8_t RtcDriver::getStatus()
{
    if (!initialized_)
    {
        return 0x02U;
    }

    return oscillatorStopped_ ? 0x01U : 0x00U;
}

uint8_t RtcDriver::bcdToDecimal(uint8_t value)
{
    return static_cast<uint8_t>(((value >> 4U) * 10U) + (value & 0x0FU));
}

uint8_t RtcDriver::decimalToBcd(uint8_t value)
{
    return static_cast<uint8_t>(((value / 10U) << 4U) | (value % 10U));
}

bool RtcDriver::validateTime(const DateTime& time) const
{
    if (time.hour > 23U || time.minute > 59U || time.second > 59U)
    {
        return false;
    }

    if (time.date < 1U || time.date > 31U)
    {
        return false;
    }

    if (time.month < 1U || time.month > 12U)
    {
        return false;
    }

    if (time.day < 1U || time.day > 7U)
    {
        return false;
    }

    return true;
}

StatusCode RtcDriver::readStatusRegister(uint8_t& value)
{
    return i2cHal_.readRegister(kRtcAddress, kRegisterStatus, value);
}

#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "hal/I2cHal.h"

struct DateTime
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
};

class RtcDriver
{
public:
    RtcDriver();

    StatusCode begin();
    StatusCode read(DateTime& time);
    StatusCode set(const DateTime& time);
    bool consumeSecondEvent();
    bool isValid();
    bool isSqwConfigured() const;
    uint32_t secondEventCount() const;
    uint8_t getStatus();

    static void onSqwInterrupt();

private:
    static constexpr uint8_t kRtcAddress = 0x68U;
    static constexpr uint8_t kRegisterSeconds = 0x00U;
    static constexpr uint8_t kRegisterControl = 0x0EU;
    static constexpr uint8_t kRegisterStatus = 0x0FU;
    static constexpr uint8_t kOscillatorStopFlag = 0x80U;

    I2cHal i2cHal_;
    StatusCode lastStatus_;
    bool initialized_;
    bool oscillatorStopped_;
    bool sqwConfigured_;

    static volatile uint8_t secondEvents_;
    static volatile uint32_t secondEventCount_;

    static uint8_t bcdToDecimal(uint8_t value);
    static uint8_t decimalToBcd(uint8_t value);
    bool validateTime(const DateTime& time) const;
    StatusCode readStatusRegister(uint8_t& value);
};

#include "drivers/ShiftRegisterDriver.h"

#include <Arduino.h>

#include "config/PinConfig.h"

namespace
{
constexpr uint8_t kBitCount = 16U;
}

StatusCode ShiftRegisterDriver::begin()
{
    pinMode(PIN_SHIFT_DATA, OUTPUT);
    pinMode(PIN_SHIFT_CLOCK, OUTPUT);
    pinMode(PIN_SHIFT_LATCH, OUTPUT);
    pinMode(PIN_DISPLAY_OE, OUTPUT);

    digitalWrite(PIN_SHIFT_DATA, LOW);
    digitalWrite(PIN_SHIFT_CLOCK, LOW);
    digitalWrite(PIN_SHIFT_LATCH, LOW);
    digitalWrite(PIN_DISPLAY_OE, LOW);

    return StatusCode::OK;
}

void ShiftRegisterDriver::shiftOut(uint8_t segmentData, uint8_t digitData)
{
    digitalWrite(PIN_SHIFT_LATCH, LOW);

    for (uint8_t bit = 0; bit < 8U; ++bit)
    {
        const uint8_t currentBit = (digitData >> bit) & 0x01U;
        digitalWrite(PIN_SHIFT_DATA, currentBit ? HIGH : LOW);
        digitalWrite(PIN_SHIFT_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_CLOCK, LOW);
    }

    for (uint8_t bit = 0; bit < 8U; ++bit)
    {
        const uint8_t currentBit = (segmentData >> bit) & 0x01U;
        digitalWrite(PIN_SHIFT_DATA, currentBit ? HIGH : LOW);
        digitalWrite(PIN_SHIFT_CLOCK, HIGH);
        digitalWrite(PIN_SHIFT_CLOCK, LOW);
    }

    digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void ShiftRegisterDriver::latch()
{
    digitalWrite(PIN_SHIFT_LATCH, HIGH);
}

void ShiftRegisterDriver::setOutputEnable(bool enable)
{
    digitalWrite(PIN_DISPLAY_OE, enable ? LOW : HIGH);
}

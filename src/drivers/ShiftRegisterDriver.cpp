#include "drivers/ShiftRegisterDriver.h"

#include <Arduino.h>
#include <avr/io.h>

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
    PORTB &= static_cast<uint8_t>(~_BV(PB2));

    for (uint8_t bit = 0; bit < 8U; ++bit)
    {
        const uint8_t currentBit = (digitData >> bit) & 0x01U;
        if (currentBit != 0U)
        {
            PORTB |= _BV(PB3);
        }
        else
        {
            PORTB &= static_cast<uint8_t>(~_BV(PB3));
        }
        PORTB |= _BV(PB5);
        PORTB &= static_cast<uint8_t>(~_BV(PB5));
    }

    for (uint8_t bit = 0; bit < 8U; ++bit)
    {
        const uint8_t currentBit = (segmentData >> bit) & 0x01U;
        if (currentBit != 0U)
        {
            PORTB |= _BV(PB3);
        }
        else
        {
            PORTB &= static_cast<uint8_t>(~_BV(PB3));
        }
        PORTB |= _BV(PB5);
        PORTB &= static_cast<uint8_t>(~_BV(PB5));
    }

    PORTB |= _BV(PB2);
}

void ShiftRegisterDriver::latch()
{
    PORTB |= _BV(PB2);
}

void ShiftRegisterDriver::setOutputEnable(bool enable)
{
    if (enable)
    {
        PORTB &= static_cast<uint8_t>(~_BV(PB1));
    }
    else
    {
        PORTB |= _BV(PB1);
    }
}

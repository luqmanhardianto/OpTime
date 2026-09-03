#include "drivers/DisplayDriver.h"

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>

#include "config/PinConfig.h"

namespace
{
DisplayDriver* gDisplayDriver = nullptr;
constexpr uint8_t kBlankDigit = 0xFFU;
// shiftOut() sends each byte LSB first, therefore byte bit 7 reaches QA and
// byte bit 0 reaches QH.  Values are written in binary and decimal to make
// the physical 74HC595 mapping easy to verify with a logic analyser.
//
// SR #2: QA=NC, QB=D6, QC=D5, QD=D4, QE=D3, QF=D2, QG=D1, QH=tick.
constexpr uint8_t kTickMask = 0b00000001U;    // QH, tick/colon,         1
constexpr uint8_t kDigit1Mask = 0b00000010U;  // QG, D1 hour tens,       2
constexpr uint8_t kDigit2Mask = 0b00000100U;  // QF, D2 hour units,      4
constexpr uint8_t kDigit3Mask = 0b00001000U;  // QE, D3 minute tens,     8
constexpr uint8_t kDigit4Mask = 0b00010000U;  // QD, D4 minute units,   16
constexpr uint8_t kDigit5Mask = 0b00100000U;  // QC, D5 second tens,    32
constexpr uint8_t kDigit6Mask = 0b01000000U;  // QB, D6 second units,   64

void initTimer1ForDisplay()
{
    noInterrupts();
    TCCR1A = 0U;
    TCCR1B = 0U;
    TCNT1 = 0U;
    // Refresh one digit every 1 ms.  The shift-register transfer uses
    // digitalWrite() in the ISR, so the former 128 us period could consume
    // nearly all CPU time and starve boot, Serial, and button handling.
    OCR1A = 249U;
    OCR1B = 186U;
    TIFR1 = _BV(OCF1A);
    TIMSK1 = _BV(OCIE1A) | _BV(OCIE1B);
    TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);
    interrupts();
}
}

ISR(TIMER1_COMPA_vect)
{
    if (gDisplayDriver != nullptr)
    {
        gDisplayDriver->refreshISR();
    }
}

ISR(TIMER1_COMPB_vect)
{
    if (gDisplayDriver != nullptr)
    {
        gDisplayDriver->refreshBrightnessISR();
    }
}

StatusCode DisplayDriver::begin()
{
    initialized_ = false;
    currentDigit_ = 0U;
    brightnessLevel_ = 75U;
    colonEnabled_ = false;

    for (uint8_t i = 0; i < kBufferSize; ++i)
    {
        frontBuffer_[i] = kBlankDigit;
        backBuffer_[i] = kBlankDigit;
    }

    shiftRegister_.begin();
    shiftRegister_.setOutputEnable(false);

    pinMode(PIN_SHIFT_DATA, OUTPUT);
    pinMode(PIN_SHIFT_CLOCK, OUTPUT);
    pinMode(PIN_SHIFT_LATCH, OUTPUT);
    pinMode(PIN_DISPLAY_OE, OUTPUT);

    digitalWrite(PIN_SHIFT_DATA, LOW);
    digitalWrite(PIN_SHIFT_CLOCK, LOW);
    digitalWrite(PIN_SHIFT_LATCH, LOW);
    digitalWrite(PIN_DISPLAY_OE, LOW);

    gDisplayDriver = this;
    TIMSK1 = 0U;
    initialized_ = true;

    clear();
    swapBuffer();
    shiftRegister_.setOutputEnable(false);
    digitalWrite(PIN_DISPLAY_OE, HIGH);
    return StatusCode::OK;
}

void DisplayDriver::enableRefresh()
{
    if (!initialized_)
    {
        return;
    }

    initTimer1ForDisplay();
}

void DisplayDriver::disableRefresh()
{
    noInterrupts();
    TIMSK1 = 0U;
    TCCR1B = 0U;
    TCNT1 = 0U;
    interrupts();
}

void DisplayDriver::setDigit(uint8_t index, uint8_t value)
{
    if (!initialized_ || index >= kBufferSize)
    {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        backBuffer_[index] = value;
    }
}

void DisplayDriver::setTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    if (!initialized_)
    {
        return;
    }

    const uint8_t hourTens = hour / 10U;
    const uint8_t hourOnes = hour % 10U;
    const uint8_t minuteTens = minute / 10U;
    const uint8_t minuteOnes = minute % 10U;
    const uint8_t secondTens = second / 10U;
    const uint8_t secondOnes = second % 10U;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        backBuffer_[0] = hourTens;
        backBuffer_[1] = hourOnes;
        backBuffer_[2] = minuteTens;
        backBuffer_[3] = minuteOnes;
        backBuffer_[4] = secondTens;
        backBuffer_[5] = secondOnes;
    }
}

void DisplayDriver::setColon(bool state)
{
    if (!initialized_)
    {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        colonEnabled_ = state;
    }
}

void DisplayDriver::clear()
{
    if (!initialized_)
    {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (uint8_t i = 0; i < kBufferSize; ++i)
        {
            backBuffer_[i] = kBlankDigit;
            frontBuffer_[i] = kBlankDigit;
        }
        colonEnabled_ = false;
    }
}

void DisplayDriver::refreshISR()
{
    if (!initialized_ || (TIMSK1 == 0U))
    {
        return;
    }

    refreshDirect();
}

void DisplayDriver::refreshDirect()
{
    if (!initialized_)
    {
        return;
    }

    uint8_t index = currentDigit_;
    if (index >= kBufferSize)
    {
        index = 0U;
        currentDigit_ = 0U;
    }

    const uint8_t digitValue = frontBuffer_[index];
    const uint8_t pattern = segmentValueForDigit(digitValue);

    uint8_t segmentData = pattern;
    uint8_t digitData = digitMaskForIndex(index);
    if (colonEnabled_)
    {
        digitData |= kTickMask;
    }

    shiftRegister_.setOutputEnable(false);
    shiftRegister_.shiftOut(segmentData, digitData);
    shiftRegister_.latch();

    if (brightnessLevel_ > 0U)
    {
        shiftRegister_.setOutputEnable(true);
    }

    if (currentDigit_ >= (kBufferSize - 1U))
    {
        currentDigit_ = 0U;
    }
    else
    {
        currentDigit_++;
    }
}

void DisplayDriver::refreshBrightnessISR()
{
    if (!initialized_ || brightnessLevel_ >= 100U)
    {
        return;
    }

    shiftRegister_.setOutputEnable(false);
}

void DisplayDriver::swapBuffer()
{
    if (!initialized_)
    {
        return;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        for (uint8_t i = 0; i < kBufferSize; ++i)
        {
            frontBuffer_[i] = backBuffer_[i];
        }
    }
}

void DisplayDriver::setBrightness(uint8_t level)
{
    if (!initialized_)
    {
        return;
    }

    brightnessLevel_ = level > 100U ? 100U : level;
}

uint8_t DisplayDriver::digitMaskForIndex(uint8_t index) const
{
    switch (index)
    {
        case 0U:
            return kDigit1Mask;
        case 1U:
            return kDigit2Mask;
        case 2U:
            return kDigit3Mask;
        case 3U:
            return kDigit4Mask;
        case 4U:
            return kDigit5Mask;
        case 5U:
            return kDigit6Mask;
        default:
            return 0x00U;
    }
}

uint8_t DisplayDriver::segmentValueForDigit(uint8_t value) const
{
    if (value > 9U)
    {
        return SegmentEncoder::blank();
    }

    return SegmentEncoder::encodeDigit(value);
}

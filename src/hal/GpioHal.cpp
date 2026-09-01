#include "hal/GpioHal.h"

#include <Arduino.h>

#include "bsp/BoardConfig.h"

StatusCode GpioHal::begin()
{
    pinModeInput(Board::Pin::RTC_SQW);
    pinModeInput(Board::Pin::BUTTON_POWER);
    pinModeInput(Board::Pin::BUTTON_SELECT);
    pinModeInput(Board::Pin::BUTTON_NEXT);
    pinModeInput(Board::Pin::BUTTON_UP);
    pinModeInput(Board::Pin::BUTTON_DOWN);

    pinModeOutput(Board::Pin::BUZZER);
    pinModeOutput(Board::Pin::DISPLAY_OE);
    pinModeOutput(Board::Pin::DISPLAY_LATCH);
    pinModeOutput(Board::Pin::DISPLAY_DATA);
    pinModeOutput(Board::Pin::POWER_LED);
    pinModeOutput(Board::Pin::DISPLAY_CLOCK);

    writeActiveLow(Board::Pin::BUZZER, false);
    writeActiveLow(Board::Pin::DISPLAY_OE, true);
    write(Board::Pin::DISPLAY_LATCH, false);
    write(Board::Pin::DISPLAY_DATA, false);
    writeActiveLow(Board::Pin::POWER_LED, false);
    write(Board::Pin::DISPLAY_CLOCK, false);

    return StatusCode::OK;
}

void GpioHal::pinModeInput(uint8_t pin)
{
    pinMode(pin, INPUT_PULLUP);
}

void GpioHal::pinModeOutput(uint8_t pin)
{
    pinMode(pin, OUTPUT);
}

bool GpioHal::read(uint8_t pin)
{
    return digitalRead(pin) == HIGH;
}

void GpioHal::write(uint8_t pin, bool state)
{
    digitalWrite(pin, state ? HIGH : LOW);
}

void GpioHal::writeActiveLow(uint8_t pin, bool enable)
{
    digitalWrite(pin, enable ? LOW : HIGH);
}

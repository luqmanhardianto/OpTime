#include "hal/TimerHal.h"

#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>

namespace
{
volatile uint32_t systemTickMs = 0U;
volatile uint32_t systemTickUs = 0U;
volatile uint8_t microsCounter = 0U;

void (*tickCallback)() = nullptr;
}

void (*TimerHal::tickCallback_)() = nullptr;

ISR(TIMER2_COMPA_vect)
{
    systemTickMs++;
    systemTickUs += 1000U;
    microsCounter += 1U;

    if (tickCallback != nullptr)
    {
        tickCallback();
    }
}

StatusCode TimerHal::begin()
{
    cli();

    TCCR2A = _BV(WGM21);
    // ATmega328P @ 16 MHz: prescaler 64 and OCR2A 249 gives exactly
    // 1,000 interrupts per second (1 ms per scheduler tick).
    TCCR2B = _BV(CS22);
    OCR2A = 249U;
    TIMSK2 = _BV(OCIE2A);

    systemTickMs = 0U;
    systemTickUs = 0U;
    microsCounter = 0U;
    tickCallback = nullptr;

    sei();
    return StatusCode::OK;
}

uint32_t TimerHal::millis()
{
    uint32_t value = 0U;
    noInterrupts();
    value = systemTickMs;
    interrupts();
    return value;
}

uint32_t TimerHal::micros()
{
    uint32_t value = 0U;
    noInterrupts();
    value = systemTickUs + (static_cast<uint32_t>(microsCounter) * 1000U);
    interrupts();
    return value;
}

void TimerHal::attachTickCallback(void (*callback)())
{
    tickCallback = callback;
}

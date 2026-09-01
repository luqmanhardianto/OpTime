#include "drivers/ButtonDriver.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t kDebounceMs = BUTTON_DEBOUNCE_MS;
constexpr uint16_t kHoldMs = BUTTON_HOLD_MS;
constexpr uint16_t kRepeatMs = BUTTON_REPEAT_MS;
}

StatusCode ButtonDriver::begin()
{
    initialized_ = false;
    eventHead_ = 0U;
    eventTail_ = 0U;
    eventCount_ = 0U;

    const StatusCode status = gpioHal_.begin();
    if (status != StatusCode::OK)
    {
        return status;
    }

    for (uint8_t i = 0; i < kButtonCount; ++i)
    {
        buttons_[i].state = STATE_RELEASED;
        buttons_[i].rawState = true;
        buttons_[i].stableState = true;
        buttons_[i].debounceStartMs = 0U;
        buttons_[i].pressStartMs = 0U;
        buttons_[i].repeatStartMs = 0U;
        buttons_[i].holdEventSent = false;
        buttons_[i].repeatEventSent = false;
    }

    initialized_ = true;
    return StatusCode::OK;
}

void ButtonDriver::update()
{
    if (!initialized_)
    {
        return;
    }

    const uint32_t now = millis();
    for (uint8_t i = 0; i < kButtonCount; ++i)
    {
        handleButton(static_cast<ButtonId>(i), isPressed(static_cast<ButtonId>(i)), now);
    }
}

bool ButtonDriver::getEvent(ButtonEvent& event)
{
    if (eventCount_ == 0U)
    {
        return false;
    }

    event = eventQueue_[eventHead_];
    eventHead_ = (eventHead_ + 1U) % kEventQueueSize;
    eventCount_--;
    return true;
}

bool ButtonDriver::anyButtonPressed() const
{
    for (uint8_t i = 0; i < kButtonCount; ++i)
    {
        if (isPressed(static_cast<ButtonId>(i)))
        {
            return true;
        }
    }
    return false;
}

bool ButtonDriver::anyNonAdjustmentButtonPressed() const
{
    return isPressed(BUTTON_POWER) || isPressed(BUTTON_SELECT) || isPressed(BUTTON_NEXT);
}

uint8_t ButtonDriver::pinForButton(ButtonId id) const
{
    switch (id)
    {
        case BUTTON_POWER:
            return PIN_BUTTON_POWER;
        case BUTTON_SELECT:
            return PIN_BUTTON_SELECT;
        case BUTTON_NEXT:
            return PIN_BUTTON_NEXT;
        case BUTTON_UP:
            return PIN_BUTTON_UP;
        case BUTTON_DOWN:
            return PIN_BUTTON_DOWN;
        default:
            return PIN_BUTTON_POWER;
    }
}

bool ButtonDriver::isPressed(ButtonId id) const
{
    GpioHal gpioHal = gpioHal_;
    return !gpioHal.read(pinForButton(id));
}

void ButtonDriver::queueEvent(const ButtonEvent& event)
{
    if (eventCount_ >= kEventQueueSize)
    {
        return;
    }

    eventQueue_[eventTail_] = event;
    eventTail_ = (eventTail_ + 1U) % kEventQueueSize;
    eventCount_++;
}

void ButtonDriver::handleButton(ButtonId id, bool pressed, uint32_t now)
{
    ButtonContext& button = buttons_[static_cast<uint8_t>(id)];

    if (button.state == STATE_RELEASED)
    {
        if (!pressed)
        {
            return;
        }

        button.rawState = true;
        button.debounceStartMs = now;
        button.state = STATE_DEBOUNCE;
        return;
    }

    if (button.state == STATE_DEBOUNCE)
    {
        if (!pressed)
        {
            button.state = STATE_RELEASED;
            button.holdEventSent = false;
            button.repeatEventSent = false;
            return;
        }

        if ((now - button.debounceStartMs) < kDebounceMs)
        {
            return;
        }

        button.stableState = true;
        button.pressStartMs = now;
        button.repeatStartMs = now;
        button.holdEventSent = false;
        button.repeatEventSent = false;
        button.state = STATE_PRESSED;
        return;
    }

    if (button.state == STATE_PRESSED)
    {
        if (!pressed)
        {
            if (!button.holdEventSent)
            {
                ButtonEvent event = {id, BUTTON_SHORT_PRESS};
                queueEvent(event);
            }
            button.state = STATE_RELEASED;
            button.holdEventSent = false;
            button.repeatEventSent = false;
            return;
        }

        if (!button.holdEventSent && ((now - button.pressStartMs) >= kHoldMs))
        {
            button.holdEventSent = true;
            ButtonEvent event = {id, BUTTON_HOLD};
            queueEvent(event);
            return;
        }

        if (button.holdEventSent && ((now - button.repeatStartMs) >= kRepeatMs))
        {
            button.repeatStartMs = now;
            ButtonEvent event = {id, BUTTON_REPEAT};
            queueEvent(event);
        }
        return;
    }

    if (button.state == STATE_HOLD)
    {
        if (!pressed)
        {
            button.state = STATE_RELEASED;
            button.holdEventSent = false;
            button.repeatEventSent = false;
            return;
        }

        if ((now - button.repeatStartMs) >= kRepeatMs)
        {
            button.repeatStartMs = now;
            ButtonEvent event = {id, BUTTON_REPEAT};
            queueEvent(event);
        }
    }
}

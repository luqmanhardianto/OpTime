#pragma once

#include <stdint.h>

#include "common/Constants.h"
#include "common/Status.h"
#include "hal/GpioHal.h"

enum ButtonId
{
    BUTTON_POWER = 0,
    BUTTON_SELECT,
    BUTTON_NEXT,
    BUTTON_UP,
    BUTTON_DOWN
};

enum ButtonEventType
{
    BUTTON_NONE = 0,
    BUTTON_SHORT_PRESS,
    BUTTON_HOLD,
    BUTTON_REPEAT
};

struct ButtonEvent
{
    ButtonId id;
    ButtonEventType type;
};

class ButtonDriver
{
public:
    StatusCode begin();
    void update();
    bool getEvent(ButtonEvent& event);
    bool anyButtonPressed() const;
    bool anyNonAdjustmentButtonPressed() const;

private:
    enum ButtonState
    {
        STATE_RELEASED = 0,
        STATE_DEBOUNCE,
        STATE_PRESSED,
        STATE_HOLD
    };

    struct ButtonContext
    {
        ButtonState state;
        bool rawState;
        bool stableState;
        uint32_t debounceStartMs;
        uint32_t pressStartMs;
        uint32_t repeatStartMs;
        bool holdEventSent;
        bool repeatEventSent;
    };

    static constexpr uint8_t kButtonCount = BUTTON_COUNT;
    static constexpr uint8_t kEventQueueSize = 5U;

    GpioHal gpioHal_;
    ButtonContext buttons_[kButtonCount];
    ButtonEvent eventQueue_[kEventQueueSize];
    uint8_t eventHead_;
    uint8_t eventTail_;
    uint8_t eventCount_;
    bool initialized_;

    uint8_t pinForButton(ButtonId id) const;
    bool isPressed(ButtonId id) const;
    void queueEvent(const ButtonEvent& event);
    void handleButton(ButtonId id, bool pressed, uint32_t now);
};

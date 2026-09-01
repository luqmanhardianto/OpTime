#pragma once

#include <stdint.h>

#include "common/Status.h"

enum class EventType : uint8_t
{
    NONE = 0,

    BUTTON_SHORT,
    BUTTON_HOLD,
    BUTTON_REPEAT,

    POWER_ON,
    POWER_OFF,

    MODE_NEXT,
    MODE_SELECT,
    MODE_CHANGE,

    TIME_TICK,
    SECOND_TICK,

    COUNTDOWN_START,
    COUNTDOWN_PAUSE,
    COUNTDOWN_RESUME,
    COUNTDOWN_RESET,
    COUNTDOWN_COMPLETED,

    TIMER_START,
    TIMER_STOP,
    TIMER_RESET,

    VALUE_UP,
    VALUE_DOWN,

    SAVE,
    RESET,

    NOTIFICATION,

    ERROR,

    SYSTEM_READY,
    SYSTEM_ERROR
};

enum class EventSource : uint8_t
{
    SYSTEM = 0,
    BUTTON,
    RTC,
    TIMER,
    UI,
    MODE,
    DIAGNOSTIC
};

struct Event
{
    EventType type;
    uint8_t source;
    int16_t value;
};

class EventSystem
{
public:
    static constexpr uint8_t EVENT_QUEUE_SIZE = 8U;
    static constexpr uint8_t MAX_EVENTS_PER_CYCLE = 4U;

    StatusCode begin();
    StatusCode publish(const Event& event);
    bool consume(Event& event);
    uint8_t pending() const;
    uint8_t overflowCount() const;
    void clear();

private:
    Event queue_[EVENT_QUEUE_SIZE];
    uint8_t head_;
    uint8_t tail_;
    uint8_t count_;
    uint8_t overflowCount_;

    static bool isCriticalEvent(EventType type);
};

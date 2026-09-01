#include "core/EventSystem.h"

#include <Arduino.h>
#include <util/atomic.h>

StatusCode EventSystem::begin()
{
    head_ = 0U;
    tail_ = 0U;
    count_ = 0U;
    overflowCount_ = 0U;

    for (uint8_t i = 0; i < EVENT_QUEUE_SIZE; ++i)
    {
        queue_[i].type = EventType::NONE;
        queue_[i].source = 0U;
        queue_[i].value = 0;
    }

    return StatusCode::OK;
}

StatusCode EventSystem::publish(const Event& event)
{
    if (event.type == EventType::NONE)
    {
        return StatusCode::INVALID_PARAMETER;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (count_ >= EVENT_QUEUE_SIZE)
        {
            if (isCriticalEvent(event.type))
            {
                overflowCount_++;
                return StatusCode::NO_RESOURCE;
            }

            overflowCount_++;
            return StatusCode::NO_RESOURCE;
        }

        queue_[tail_] = event;
        tail_ = (tail_ + 1U) % EVENT_QUEUE_SIZE;
        count_++;
    }

    return StatusCode::OK;
}

bool EventSystem::consume(Event& event)
{
    if (count_ == 0U)
    {
        return false;
    }

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        event = queue_[head_];
        queue_[head_].type = EventType::NONE;
        queue_[head_].source = 0U;
        queue_[head_].value = 0;
        head_ = (head_ + 1U) % EVENT_QUEUE_SIZE;
        count_--;
    }

    return true;
}

uint8_t EventSystem::pending() const
{
    return count_;
}

uint8_t EventSystem::overflowCount() const
{
    return overflowCount_;
}

void EventSystem::clear()
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        head_ = 0U;
        tail_ = 0U;
        count_ = 0U;
    }
}

bool EventSystem::isCriticalEvent(EventType type)
{
    return type == EventType::ERROR || type == EventType::SYSTEM_ERROR ||
           type == EventType::POWER_OFF || type == EventType::RESET;
}

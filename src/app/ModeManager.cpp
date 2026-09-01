#include "app/ModeManager.h"

namespace ot
{
namespace
{
EventSystem gDefaultEventSystem;
}

ModeManager::ModeManager()
    : eventSystem_(nullptr),
      currentMode_(AppMode::CLOCK),
      previousMode_(AppMode::CLOCK)
{
}

StatusCode ModeManager::begin()
{
    eventSystem_ = &gDefaultEventSystem;
    eventSystem_->begin();
    currentMode_ = AppMode::CLOCK;
    previousMode_ = AppMode::CLOCK;
    return StatusCode::OK;
}

StatusCode ModeManager::begin(EventSystem* eventSystem)
{
    eventSystem_ = eventSystem != nullptr ? eventSystem : &gDefaultEventSystem;
    eventSystem_->begin();
    currentMode_ = AppMode::CLOCK;
    previousMode_ = AppMode::CLOCK;
    return StatusCode::OK;
}

void ModeManager::update()
{
    if (eventSystem_ == nullptr)
    {
        return;
    }

    Event event;
    while (eventSystem_->consume(event))
    {
        handleEvent(event);
    }
}

StatusCode ModeManager::setMode(AppMode mode)
{
    if (!isNormalMode(mode))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    if (currentMode_ == mode)
    {
        return StatusCode::NO_CHANGE;
    }

    if (!isValidTransition(currentMode_, mode))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    const AppMode previous = currentMode_;
    previousMode_ = currentMode_;
    currentMode_ = mode;
    publishModeChangeEvent(previous, currentMode_, EventSource::MODE);
    return StatusCode::OK;
}

StatusCode ModeManager::setMode(const ModeChangeRequest& request)
{
    if (!isNormalMode(request.targetMode))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    return setMode(request.targetMode);
}

StatusCode ModeManager::nextMode()
{
    const AppMode next = nextModeFor(currentMode_);
    if (next == currentMode_)
    {
        return StatusCode::NO_CHANGE;
    }

    return setMode(next);
}

AppMode ModeManager::currentMode() const
{
    return currentMode_;
}

AppMode ModeManager::previousMode() const
{
    return previousMode_;
}

bool ModeManager::isMode(AppMode mode) const
{
    return currentMode_ == mode;
}

bool ModeManager::isNormalMode(AppMode mode)
{
    switch (mode)
    {
        case AppMode::CLOCK:
        case AppMode::STOPWATCH:
        case AppMode::COUNTDOWN:
        case AppMode::FACTORY:
        case AppMode::DIAGNOSTIC:
            return true;
        default:
            return false;
    }
}

bool ModeManager::isValidTransition(AppMode current, AppMode target)
{
    switch (current)
    {
        case AppMode::CLOCK:
            return target == AppMode::STOPWATCH || target == AppMode::COUNTDOWN || target == AppMode::FACTORY;
        case AppMode::STOPWATCH:
            return target == AppMode::COUNTDOWN || target == AppMode::CLOCK || target == AppMode::FACTORY;
        case AppMode::COUNTDOWN:
            return target == AppMode::CLOCK || target == AppMode::STOPWATCH || target == AppMode::FACTORY;
        case AppMode::FACTORY:
            return target == AppMode::CLOCK || target == AppMode::STOPWATCH || target == AppMode::COUNTDOWN;
        case AppMode::DIAGNOSTIC:
            return target == AppMode::CLOCK || target == AppMode::STOPWATCH || target == AppMode::COUNTDOWN || target == AppMode::FACTORY;
        default:
            return false;
    }
}

AppMode ModeManager::nextModeFor(AppMode current)
{
    switch (current)
    {
        case AppMode::CLOCK:
            return AppMode::STOPWATCH;
        case AppMode::STOPWATCH:
            return AppMode::COUNTDOWN;
        case AppMode::COUNTDOWN:
            return AppMode::CLOCK;
        case AppMode::FACTORY:
            return AppMode::CLOCK;
        case AppMode::DIAGNOSTIC:
            return AppMode::CLOCK;
        default:
            return AppMode::CLOCK;
    }
}

uint16_t ModeManager::encodeTransition(AppMode previous, AppMode current)
{
    return static_cast<uint16_t>((static_cast<uint16_t>(previous) << 8U) |
                                 static_cast<uint16_t>(current));
}

void ModeManager::publishModeChangeEvent(AppMode previous, AppMode current, EventSource source)
{
    if (eventSystem_ == nullptr)
    {
        return;
    }

    Event event;
    event.type = EventType::MODE_CHANGE;
    event.source = static_cast<uint8_t>(source);
    event.value = static_cast<int16_t>(encodeTransition(previous, current));
    (void)eventSystem_->publish(event);
}

void ModeManager::handleEvent(const Event& event)
{
    if (event.type == EventType::MODE_NEXT)
    {
        (void)nextMode();
        return;
    }

    if (event.type == EventType::MODE_SELECT)
    {
        const AppMode target = static_cast<AppMode>(event.value);
        (void)setMode(target);
        return;
    }
}
}  // namespace ot

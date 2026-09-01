#include "services/TimeService.h"

#include <Arduino.h>

namespace
{
constexpr uint8_t kMaxHour = 99U;
constexpr uint8_t kMaxMinute = 59U;
constexpr uint8_t kMaxSecond = 59U;
}

TimeService::TimeService()
    : rtcDriver_(nullptr),
      scheduler_(nullptr),
      eventSystem_(nullptr),
      currentTime_{0U, 0U, 0U, 0U, 1U, 1U, 2000U},
      rtcValid_(false),
      monotonicMs_(0U),
      lastRtcSyncMs_(0U),
      lastSecondTickMs_(0U),
      stopwatchStartMs_(0U),
      stopwatchAccumulatedMs_(0U),
      stopwatchElapsedMs_(0U),
      stopwatchLastSecondBoundary_(0U),
      stopwatchLastTickMs_(0U),
      stopwatchState_(StopwatchState::STOPPED),
      countdownInitialMs_(0U),
      countdownRemainingMs_(0U),
      countdownBaseMs_(0U),
      countdownStartMs_(0U),
      countdownLastTickMs_(0U),
      countdownState_(CountdownState::IDLE)
{
}

StatusCode TimeService::begin(RtcDriver* rtcDriver, Scheduler* scheduler, EventSystem* eventSystem)
{
    rtcDriver_ = rtcDriver;
    scheduler_ = scheduler;
    eventSystem_ = eventSystem;

    rtcValid_ = false;
    monotonicMs_ = 0U;
    lastRtcSyncMs_ = 0U;
    lastSecondTickMs_ = 0U;

    stopwatchReset();
    countdownReset();

    if (rtcDriver_ != nullptr)
    {
        syncRtc();
    }

    return StatusCode::OK;
}

void TimeService::update()
{
    if (scheduler_ == nullptr)
    {
        return;
    }

    monotonicMs_ = scheduler_->tick();

    if (rtcDriver_ != nullptr && (uint32_t)(monotonicMs_ - lastRtcSyncMs_) >= kRtcSyncIntervalMs)
    {
        lastRtcSyncMs_ = monotonicMs_;
        syncRtc();
    }

    while ((uint32_t)(monotonicMs_ - lastSecondTickMs_) >= kSecondTickMs)
    {
        lastSecondTickMs_ += kSecondTickMs;
        publishEvent(EventType::SECOND_TICK, EventSource::RTC, static_cast<int16_t>(monotonicMs_));
    }

    refreshStopwatchState();
    refreshCountdownState();
}

StatusCode TimeService::setDateTime(const DateTime& time)
{
    if (!isValidDateTime(time))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    if (rtcDriver_ == nullptr)
    {
        return StatusCode::NOT_READY;
    }

    const StatusCode status = rtcDriver_->set(time);
    if (status == StatusCode::OK)
    {
        currentTime_ = time;
        rtcValid_ = true;
    }
    return status;
}

StatusCode TimeService::getDateTime(DateTime& time) const
{
    time = currentTime_;
    return StatusCode::OK;
}

bool TimeService::isRtcValid() const
{
    return rtcValid_;
}

uint32_t TimeService::nowMs() const
{
    if (scheduler_ != nullptr)
    {
        return scheduler_->tick();
    }
    return monotonicMs_;
}

StatusCode TimeService::stopwatchStart()
{
    if (scheduler_ == nullptr)
    {
        return StatusCode::NOT_READY;
    }

    if (stopwatchState_ == StopwatchState::RUNNING)
    {
        return StatusCode::BUSY;
    }

    if (stopwatchState_ == StopwatchState::COMPLETED)
    {
        stopwatchReset();
    }

    stopwatchStartMs_ = nowMs();
    stopwatchLastSecondBoundary_ = 0U;
    stopwatchLastTickMs_ = lastSecondTickMs_;
    if (stopwatchState_ == StopwatchState::PAUSED)
    {
        stopwatchState_ = StopwatchState::RUNNING;
        return StatusCode::OK;
    }

    stopwatchAccumulatedMs_ = 0U;
    stopwatchElapsedMs_ = 0U;
    stopwatchState_ = StopwatchState::RUNNING;
    return StatusCode::OK;
}

StatusCode TimeService::stopwatchStop()
{
    refreshStopwatchState();
    if (stopwatchState_ == StopwatchState::STOPPED || stopwatchState_ == StopwatchState::COMPLETED)
    {
        return StatusCode::NOT_READY;
    }

    stopwatchAccumulatedMs_ = stopwatchElapsedMs_;
    stopwatchState_ = StopwatchState::STOPPED;
    return StatusCode::OK;
}

StatusCode TimeService::stopwatchPause()
{
    refreshStopwatchState();
    if (stopwatchState_ != StopwatchState::RUNNING)
    {
        return StatusCode::NOT_READY;
    }

    stopwatchAccumulatedMs_ = stopwatchElapsedMs_;
    stopwatchState_ = StopwatchState::PAUSED;
    return StatusCode::OK;
}

StatusCode TimeService::stopwatchReset()
{
    stopwatchStartMs_ = 0U;
    stopwatchAccumulatedMs_ = 0U;
    stopwatchElapsedMs_ = 0U;
    stopwatchLastSecondBoundary_ = 0U;
    stopwatchLastTickMs_ = lastSecondTickMs_;
    stopwatchState_ = StopwatchState::STOPPED;
    return StatusCode::OK;
}

StatusCode TimeService::getStopwatch(TimeValue& value) const
{
    TimeService* mutableThis = const_cast<TimeService*>(this);
    mutableThis->refreshStopwatchState();

    uint32_t seconds = stopwatchElapsedMs_ / 1000U;
    if (seconds > kStopwatchMaxSeconds)
    {
        seconds = kStopwatchMaxSeconds;
    }

    secondsToTimeValue(seconds, value);
    return StatusCode::OK;
}

StopwatchState TimeService::stopwatchState() const
{
    return stopwatchState_;
}

StatusCode TimeService::countdownSet(const TimeValue& value)
{
    if (!isValidTimeValue(value))
    {
        return StatusCode::INVALID_PARAMETER;
    }

    countdownInitialMs_ = timeValueToSeconds(value) * 1000U;
    countdownRemainingMs_ = countdownInitialMs_;
    countdownBaseMs_ = countdownInitialMs_;
    countdownStartMs_ = nowMs();
    countdownLastTickMs_ = lastSecondTickMs_;
    countdownState_ = CountdownState::IDLE;
    return StatusCode::OK;
}

StatusCode TimeService::countdownStart()
{
    if (countdownState_ == CountdownState::RUNNING)
    {
        return StatusCode::BUSY;
    }

    // A completed countdown keeps its configured duration. Starting it again
    // restarts from that saved value; only countdownReset() clears it.
    if (countdownState_ == CountdownState::COMPLETED)
    {
        countdownRemainingMs_ = countdownInitialMs_;
    }

    if (countdownInitialMs_ == 0U)
    {
        countdownState_ = CountdownState::IDLE;
        return StatusCode::OK;
    }

    countdownBaseMs_ = countdownRemainingMs_;
    countdownStartMs_ = nowMs();
    countdownLastTickMs_ = lastSecondTickMs_;
    countdownState_ = CountdownState::RUNNING;
    return StatusCode::OK;
}

StatusCode TimeService::countdownStop()
{
    if (countdownState_ == CountdownState::IDLE)
    {
        return StatusCode::NOT_READY;
    }

    refreshCountdownState();
    countdownState_ = CountdownState::IDLE;
    return StatusCode::OK;
}

StatusCode TimeService::countdownPause()
{
    refreshCountdownState();
    if (countdownState_ != CountdownState::RUNNING)
    {
        return StatusCode::NOT_READY;
    }

    countdownState_ = CountdownState::PAUSED;
    return StatusCode::OK;
}

StatusCode TimeService::countdownReset()
{
    countdownInitialMs_ = 0U;
    countdownRemainingMs_ = 0U;
    countdownBaseMs_ = 0U;
    countdownStartMs_ = 0U;
    countdownLastTickMs_ = lastSecondTickMs_;
    countdownState_ = CountdownState::IDLE;
    return StatusCode::OK;
}

StatusCode TimeService::getCountdown(TimeValue& value) const
{
    TimeService* mutableThis = const_cast<TimeService*>(this);
    mutableThis->refreshCountdownState();

    uint32_t seconds = countdownRemainingMs_ / 1000U;
    secondsToTimeValue(seconds, value);
    return StatusCode::OK;
}

CountdownState TimeService::countdownState() const
{
    return countdownState_;
}

uint32_t TimeService::timeValueToSeconds(const TimeValue& value)
{
    return (static_cast<uint32_t>(value.hour) * 3600U) +
           (static_cast<uint32_t>(value.minute) * 60U) +
           static_cast<uint32_t>(value.second);
}

void TimeService::secondsToTimeValue(uint32_t seconds, TimeValue& value)
{
    value.hour = static_cast<uint8_t>((seconds / 3600U) % 100U);
    value.minute = static_cast<uint8_t>((seconds % 3600U) / 60U);
    value.second = static_cast<uint8_t>(seconds % 60U);
}

bool TimeService::isValidTimeValue(const TimeValue& value)
{
    if (value.hour > kMaxHour || value.minute > kMaxMinute || value.second > kMaxSecond)
    {
        return false;
    }

    if (value.minute > 59U || value.second > 59U)
    {
        return false;
    }

    return true;
}

bool TimeService::isValidDateTime(const DateTime& time)
{
    if (time.hour > 23U || time.minute > 59U || time.second > 59U)
    {
        return false;
    }

    if (time.date < 1U || time.date > 31U)
    {
        return false;
    }

    if (time.month < 1U || time.month > 12U)
    {
        return false;
    }

    if (time.day < 1U || time.day > 7U)
    {
        return false;
    }

    return true;
}

void TimeService::syncRtc()
{
    if (rtcDriver_ == nullptr)
    {
        rtcValid_ = false;
        return;
    }

    const StatusCode status = rtcDriver_->read(currentTime_);
    rtcValid_ = (status == StatusCode::OK) && rtcDriver_->isValid();
    if (!rtcValid_)
    {
        currentTime_ = currentTime_;
    }
}

void TimeService::publishEvent(EventType type, EventSource source, int16_t value)
{
    if (eventSystem_ == nullptr)
    {
        return;
    }

    Event event;
    event.type = type;
    event.source = static_cast<uint8_t>(source);
    event.value = value;
    (void)eventSystem_->publish(event);
}

void TimeService::refreshStopwatchState()
{
    if (stopwatchState_ != StopwatchState::RUNNING)
    {
        stopwatchElapsedMs_ = stopwatchAccumulatedMs_;
        return;
    }

    // Advance only when TimeService creates its one-second tick. This keeps
    // stopwatch/countdown changes phase-locked to the front-panel tick LED.
    while ((uint32_t)(lastSecondTickMs_ - stopwatchLastTickMs_) >= kSecondTickMs)
    {
        stopwatchLastTickMs_ += kSecondTickMs;
        stopwatchAccumulatedMs_ += kSecondTickMs;
    }
    stopwatchElapsedMs_ = stopwatchAccumulatedMs_;

    if (stopwatchElapsedMs_ >= kStopwatchMaxMs)
    {
        stopwatchElapsedMs_ = kStopwatchMaxMs;
        stopwatchAccumulatedMs_ = stopwatchElapsedMs_;
        stopwatchState_ = StopwatchState::COMPLETED;
    }
}

void TimeService::refreshCountdownState()
{
    if (countdownState_ != CountdownState::RUNNING)
    {
        return;
    }

    while ((uint32_t)(lastSecondTickMs_ - countdownLastTickMs_) >= kSecondTickMs)
    {
        countdownLastTickMs_ += kSecondTickMs;
        if (countdownRemainingMs_ <= kSecondTickMs)
        {
            countdownRemainingMs_ = 0U;
            countdownState_ = CountdownState::COMPLETED;
            publishEvent(EventType::TIMER_STOP, EventSource::TIMER, 0);
            return;
        }

        countdownRemainingMs_ -= kSecondTickMs;
    }
}

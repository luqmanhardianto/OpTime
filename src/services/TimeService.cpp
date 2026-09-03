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
    rtcEpochSeconds_(0U),
    rtcReferenceValid_(false),
    stopwatchStartEpochSeconds_(0U),
    stopwatchAccumulatedSeconds_(0U),
    stopwatchElapsedSeconds_(0U),
      stopwatchState_(StopwatchState::STOPPED),
    countdownInitialSeconds_(0U),
    countdownRemainingSeconds_(0U),
    countdownBaseSeconds_(0U),
    countdownStartEpochSeconds_(0U),
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
    rtcEpochSeconds_ = 0U;
    rtcReferenceValid_ = false;

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

    bool rtcSecondReceived = false;
    while (rtcDriver_ != nullptr && rtcDriver_->consumeSecondEvent())
    {
        rtcSecondReceived = true;
    }

    if (rtcSecondReceived)
    {
        onRtcSecond();
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
        rtcEpochSeconds_ = dateTimeToEpochSeconds(time);
        rtcReferenceValid_ = true;
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
    return ::millis();
}

StatusCode TimeService::stopwatchStart()
{
    if (stopwatchState_ == StopwatchState::RUNNING)
    {
        return StatusCode::BUSY;
    }

    if (stopwatchState_ == StopwatchState::COMPLETED)
    {
        stopwatchReset();
    }

    syncRtc();
    if (!rtcReferenceValid_)
    {
        return StatusCode::NOT_READY;
    }

    stopwatchStartEpochSeconds_ = rtcEpochSeconds_;
    if (stopwatchState_ == StopwatchState::PAUSED)
    {
        stopwatchState_ = StopwatchState::RUNNING;
        return StatusCode::OK;
    }

    stopwatchAccumulatedSeconds_ = 0U;
    stopwatchElapsedSeconds_ = 0U;
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

    syncRtc();
    refreshStopwatchState();
    stopwatchAccumulatedSeconds_ = stopwatchElapsedSeconds_;
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

    syncRtc();
    refreshStopwatchState();
    stopwatchAccumulatedSeconds_ = stopwatchElapsedSeconds_;
    stopwatchState_ = StopwatchState::PAUSED;
    return StatusCode::OK;
}

StatusCode TimeService::stopwatchReset()
{
    stopwatchStartEpochSeconds_ = 0U;
    stopwatchAccumulatedSeconds_ = 0U;
    stopwatchElapsedSeconds_ = 0U;
    stopwatchState_ = StopwatchState::STOPPED;
    return StatusCode::OK;
}

StatusCode TimeService::getStopwatch(TimeValue& value) const
{
    TimeService* mutableThis = const_cast<TimeService*>(this);
    mutableThis->refreshStopwatchState();

    uint32_t seconds = stopwatchElapsedSeconds_;
    if (seconds > kStopwatchMaxSeconds)
    {
        seconds = kStopwatchMaxSeconds;
    }

    secondsToTimeValue(seconds, value);
    return StatusCode::OK;
}

uint32_t TimeService::stopwatchElapsedSeconds() const
{
    TimeService* mutableThis = const_cast<TimeService*>(this);
    mutableThis->refreshStopwatchState();
    return stopwatchElapsedSeconds_;
}

void TimeService::onRtcSecond()
{
    syncRtc();
    publishEvent(EventType::SECOND_TICK, EventSource::RTC,
                 static_cast<int16_t>(rtcEpochSeconds_ & 0x7FFFU));
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

    countdownInitialSeconds_ = timeValueToSeconds(value);
    countdownRemainingSeconds_ = countdownInitialSeconds_;
    countdownBaseSeconds_ = countdownInitialSeconds_;
    countdownStartEpochSeconds_ = rtcEpochSeconds_;
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
        countdownRemainingSeconds_ = countdownInitialSeconds_;
    }

    syncRtc();
    if (!rtcReferenceValid_)
    {
        return StatusCode::NOT_READY;
    }

    if (countdownInitialSeconds_ == 0U)
    {
        countdownState_ = CountdownState::IDLE;
        return StatusCode::OK;
    }

    countdownBaseSeconds_ = countdownRemainingSeconds_;
    countdownStartEpochSeconds_ = rtcEpochSeconds_;
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
    countdownInitialSeconds_ = 0U;
    countdownRemainingSeconds_ = 0U;
    countdownBaseSeconds_ = 0U;
    countdownStartEpochSeconds_ = 0U;
    countdownState_ = CountdownState::IDLE;
    return StatusCode::OK;
}

StatusCode TimeService::getCountdown(TimeValue& value) const
{
    TimeService* mutableThis = const_cast<TimeService*>(this);
    mutableThis->refreshCountdownState();

    uint32_t seconds = countdownRemainingSeconds_;
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

uint32_t TimeService::dateTimeToEpochSeconds(const DateTime& time)
{
    uint32_t days = 0U;
    for (uint16_t year = 2000U; year < time.year; ++year)
    {
        days += ((year % 4U) == 0U) ? 366U : 365U;
    }

    static const uint8_t daysInMonth[] = {31U, 28U, 31U, 30U, 31U, 30U,
                                          31U, 31U, 30U, 31U, 30U, 31U};
    for (uint8_t month = 1U; month < time.month; ++month)
    {
        days += daysInMonth[month - 1U];
        if (month == 2U && ((time.year % 4U) == 0U))
        {
            days++;
        }
    }

    days += static_cast<uint32_t>(time.date - 1U);
    return (days * 86400U) +
           (static_cast<uint32_t>(time.hour) * 3600U) +
           (static_cast<uint32_t>(time.minute) * 60U) +
           static_cast<uint32_t>(time.second);
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
    if (rtcValid_)
    {
        rtcEpochSeconds_ = dateTimeToEpochSeconds(currentTime_);
        rtcReferenceValid_ = true;
    }
    else
    {
        rtcReferenceValid_ = false;
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
        stopwatchElapsedSeconds_ = stopwatchAccumulatedSeconds_;
        return;
    }

    if (!rtcReferenceValid_ || rtcEpochSeconds_ < stopwatchStartEpochSeconds_)
    {
        return;
    }

    stopwatchElapsedSeconds_ = stopwatchAccumulatedSeconds_ +
                               (rtcEpochSeconds_ - stopwatchStartEpochSeconds_);

    if (stopwatchElapsedSeconds_ >= kStopwatchMaxSeconds)
    {
        stopwatchElapsedSeconds_ = kStopwatchMaxSeconds;
        stopwatchAccumulatedSeconds_ = stopwatchElapsedSeconds_;
        stopwatchState_ = StopwatchState::COMPLETED;
    }
}

void TimeService::refreshCountdownState()
{
    if (countdownState_ != CountdownState::RUNNING)
    {
        return;
    }

    if (!rtcReferenceValid_ || rtcEpochSeconds_ < countdownStartEpochSeconds_)
    {
        return;
    }

    const uint32_t elapsedSeconds = rtcEpochSeconds_ - countdownStartEpochSeconds_;
    if (elapsedSeconds >= countdownBaseSeconds_)
    {
        countdownRemainingSeconds_ = 0U;
        countdownState_ = CountdownState::COMPLETED;
        publishEvent(EventType::TIMER_STOP, EventSource::TIMER, 0);
        return;
    }

    countdownRemainingSeconds_ = countdownBaseSeconds_ - elapsedSeconds;
}

#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "core/EventSystem.h"
#include "drivers/RtcDriver.h"
#include "scheduler/Scheduler.h"

struct TimeValue
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

enum class StopwatchState : uint8_t
{
    STOPPED = 0,
    RUNNING,
    PAUSED,
    COMPLETED
};

enum class CountdownState : uint8_t
{
    IDLE = 0,
    RUNNING,
    PAUSED,
    COMPLETED
};

class TimeService
{
public:
    TimeService();

    StatusCode begin(RtcDriver* rtcDriver, Scheduler* scheduler, EventSystem* eventSystem);
    void update();

    StatusCode setDateTime(const DateTime& time);
    StatusCode getDateTime(DateTime& time) const;
    bool isRtcValid() const;
    uint32_t nowMs() const;

    StatusCode stopwatchStart();
    StatusCode stopwatchStop();
    StatusCode stopwatchPause();
    StatusCode stopwatchReset();
    StatusCode getStopwatch(TimeValue& value) const;
    uint32_t stopwatchElapsedSeconds() const;
    void onRtcSecond();
    StopwatchState stopwatchState() const;

    StatusCode countdownSet(const TimeValue& value);
    StatusCode countdownStart();
    StatusCode countdownStop();
    StatusCode countdownPause();
    StatusCode countdownReset();
    StatusCode getCountdown(TimeValue& value) const;
    CountdownState countdownState() const;

private:
    static constexpr uint32_t kRtcSyncIntervalMs = 1000U;
    static constexpr uint32_t kSecondTickMs = 1000U;
    static constexpr uint32_t kStopwatchMaxSeconds = ((99U * 3600U) + (59U * 60U) + 59U);
    static uint32_t timeValueToSeconds(const TimeValue& value);
    static void secondsToTimeValue(uint32_t seconds, TimeValue& value);
    static uint32_t dateTimeToEpochSeconds(const DateTime& time);
    static bool isValidTimeValue(const TimeValue& value);
    static bool isValidDateTime(const DateTime& time);

    void syncRtc();
    void publishEvent(EventType type, EventSource source, int16_t value);
    void refreshStopwatchState();
    void refreshCountdownState();

    RtcDriver* rtcDriver_;
    Scheduler* scheduler_;
    EventSystem* eventSystem_;
    DateTime currentTime_;
    bool rtcValid_;
    uint32_t monotonicMs_;
    uint32_t lastRtcSyncMs_;
    uint32_t rtcEpochSeconds_;
    bool rtcReferenceValid_;

    uint32_t stopwatchStartEpochSeconds_;
    uint32_t stopwatchAccumulatedSeconds_;
    uint32_t stopwatchElapsedSeconds_;
    StopwatchState stopwatchState_;

    uint32_t countdownInitialSeconds_;
    uint32_t countdownRemainingSeconds_;
    uint32_t countdownBaseSeconds_;
    uint32_t countdownStartEpochSeconds_;
    CountdownState countdownState_;
};

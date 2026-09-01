#pragma once

#include <stdint.h>

#include "common/Status.h"
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

    StatusCode begin(RtcDriver& rtcDriver, Scheduler& scheduler);
    void update();

    StatusCode setDateTime(const DateTime& time);
    StatusCode getDateTime(DateTime& time) const;
    bool isRtcValid() const;

    StatusCode stopwatchStart();
    StatusCode stopwatchStop();
    StatusCode stopwatchPause();
    StatusCode stopwatchReset();
    StatusCode getStopwatch(TimeValue& value) const;
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
    static constexpr uint32_t kStopwatchMaxMs = kStopwatchMaxSeconds * 1000U;

    static uint32_t timeValueToSeconds(const TimeValue& value);
    static void secondsToTimeValue(uint32_t seconds, TimeValue& value);
    static bool isValidTimeValue(const TimeValue& value);
    static bool isValidDateTime(const DateTime& time);

    void syncRtc();
    void refreshStopwatchState();
    void refreshCountdownState();

    RtcDriver* rtcDriver_;
    Scheduler* scheduler_;
    DateTime currentTime_;
    bool rtcValid_;
    uint32_t monotonicMs_;
    uint32_t lastRtcSyncMs_;
    uint32_t lastSecondTickMs_;

    uint32_t stopwatchAccumulatedMs_;
    uint32_t stopwatchElapsedMs_;
    uint32_t stopwatchLastTickMs_;
    StopwatchState stopwatchState_;

    uint32_t countdownInitialMs_;
    uint32_t countdownRemainingMs_;
    uint32_t countdownLastTickMs_;
    CountdownState countdownState_;
};

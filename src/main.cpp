#include "Arduino.h"
#include <avr/io.h>
#include <stdlib.h>

#include "app/ModeManager.h"
#include "bsp/BoardConfig.h"
#include "config/DiagnosticConfig.h"
#include "core/EventSystem.h"
#include "drivers/ButtonDriver.h"
#include "drivers/DisplayDriver.h"
#include "drivers/RtcDriver.h"
#include "scheduler/Scheduler.h"
#include "services/TimeService.h"

#if (OPTIME_TIMER1_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL) || \
    (OPTIME_MILLIS_RTC_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL)
class DiagnosticSerial
{
public:
    void begin(const uint32_t baudRate)
    {
        const uint16_t divisor = static_cast<uint16_t>((F_CPU / (16UL * baudRate)) - 1UL);
        UBRR0H = static_cast<uint8_t>(divisor >> 8U);
        UBRR0L = static_cast<uint8_t>(divisor & 0xFFU);
        UCSR0A = 0U;
        UCSR0B = _BV(TXEN0);
        UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
    }

    void print(const char* text)
    {
        while (*text != '\0')
        {
            writeByte(*text++);
        }
    }

    void print(const uint32_t value)
    {
        char buffer[11];
        utoa(value, buffer, 10);
        print(buffer);
    }

    void print(const uint8_t value)
    {
        print(static_cast<uint32_t>(value));
    }

    void print(const uint16_t value)
    {
        print(static_cast<uint32_t>(value));
    }

    void print(const int32_t value)
    {
        char buffer[12];
        ltoa(value, buffer, 10);
        print(buffer);
    }

    void println()
    {
        print("\r\n");
    }

    void println(const char* text)
    {
        print(text);
        println();
    }

    void println(const uint32_t value)
    {
        print(value);
        println();
    }

    void println(const uint8_t value)
    {
        print(value);
        println();
    }

    void println(const uint16_t value)
    {
        print(value);
        println();
    }

    void println(const int32_t value)
    {
        print(value);
        println();
    }

private:
    void writeByte(const char value)
    {
        while ((UCSR0A & _BV(UDRE0)) == 0U)
        {
        }
        UDR0 = static_cast<uint8_t>(value);
    }
};
#endif

class DisabledSerial
{
public:
    void begin(const uint32_t) {}

    template <typename ValueType>
    void print(const ValueType&) {}

    void println() {}

    template <typename ValueType>
    void println(const ValueType&) {}
};

namespace
{
#if (OPTIME_TIMER1_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL) || \
    (OPTIME_MILLIS_RTC_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL)
DiagnosticSerial gDiagnosticSerial;
#else
DisabledSerial gDisabledSerial;
#endif
const uint8_t kPowerLedPin = Board::Pin::POWER_LED;
const uint8_t kBuzzerPin = Board::Pin::BUZZER;
constexpr uint32_t kCountdownCompletionToneMs = 3000U;
constexpr uint8_t kDisplayBlankDigit = 0xFFU;
constexpr uint32_t kEditFieldBlinkPeriodMs = 500U;
TimeService gTimeService;
#if (OPTIME_TIMER1_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL) || \
    (OPTIME_MILLIS_RTC_DIAGNOSTIC && OPTIME_TIMER1_DIAGNOSTIC_SERIAL)
#define Serial gDiagnosticSerial
#else
#define Serial gDisabledSerial
#endif

const char* modeName(const ot::AppMode mode)
{
    switch (mode)
    {
        case ot::AppMode::CLOCK:
            return "CLOCK";
        case ot::AppMode::STOPWATCH:
            return "STOPWATCH";
        case ot::AppMode::COUNTDOWN:
            return "COUNTDOWN";
        case ot::AppMode::FACTORY:
            return "FACTORY";
        case ot::AppMode::DIAGNOSTIC:
            return "DIAGNOSTIC";
        default:
            return "UNKNOWN";
    }
}

const char* buttonName(const ButtonId id)
{
    switch (id)
    {
        case BUTTON_POWER:
            return "POWER";
        case BUTTON_SELECT:
            return "SELECT";
        case BUTTON_NEXT:
            return "NEXT";
        case BUTTON_UP:
            return "UP";
        case BUTTON_DOWN:
            return "DOWN";
        default:
            return "UNKNOWN";
    }
}

const char* buttonEventName(const ButtonEventType type)
{
    switch (type)
    {
        case BUTTON_SHORT_PRESS:
            return "SHORT_PRESS";
        case BUTTON_HOLD:
            return "HOLD";
        case BUTTON_REPEAT:
            return "REPEAT";
        default:
            return "NONE";
    }
}

void printDateTime(const DateTime& time)
{
    Serial.print(time.year);
    Serial.print("-");
    Serial.print(time.month < 10U ? "0" : "");
    Serial.print(time.month);
    Serial.print("-");
    Serial.print(time.date < 10U ? "0" : "");
    Serial.print(time.date);
    Serial.print(" ");
    Serial.print(time.hour < 10U ? "0" : "");
    Serial.print(time.hour);
    Serial.print(":");
    Serial.print(time.minute < 10U ? "0" : "");
    Serial.print(time.minute);
    Serial.print(":");
    Serial.print(time.second < 10U ? "0" : "");
    Serial.print(time.second);
}

void printTimeHhMmSs(const DateTime& time)
{
    Serial.print(time.hour < 10U ? "0" : "");
    Serial.print(time.hour);
    Serial.print(":");
    Serial.print(time.minute < 10U ? "0" : "");
    Serial.print(time.minute);
    Serial.print(":");
    Serial.print(time.second < 10U ? "0" : "");
    Serial.print(time.second);
}

void printStopwatchValue(const TimeValue& value)
{
    Serial.print(value.hour < 10U ? "0" : "");
    Serial.print(value.hour);
    Serial.print(":");
    Serial.print(value.minute < 10U ? "0" : "");
    Serial.print(value.minute);
    Serial.print(":");
    Serial.print(value.second < 10U ? "0" : "");
    Serial.print(value.second);
}

const char* stopwatchStateName(const StopwatchState state)
{
    switch (state)
    {
        case StopwatchState::STOPPED:
            return "STOPPED";
        case StopwatchState::RUNNING:
            return "RUNNING";
        case StopwatchState::PAUSED:
            return "PAUSED";
        default:
            return "UNKNOWN";
    }
}

const char* countdownStateName(const CountdownState state)
{
    switch (state)
    {
        case CountdownState::IDLE:
            return "IDLE";
        case CountdownState::RUNNING:
            return "RUNNING";
        case CountdownState::PAUSED:
            return "PAUSED";
        case CountdownState::COMPLETED:
            return "COMPLETED";
        default:
            return "UNKNOWN";
    }
}

void beepOnce(uint16_t durationMs)
{
    digitalWrite(kBuzzerPin, LOW);
    delay(durationMs);
    digitalWrite(kBuzzerPin, HIGH);
}

enum class ClockEditField : uint8_t
{
    HOUR = 0,
    MINUTE = 1,
    SECOND = 2
};

bool gClockEditing = false;
ClockEditField gClockField = ClockEditField::HOUR;
DateTime gEditTime = {0U, 0U, 0U, 0U, 1U, 1U, 2000U};

enum class CountdownEditField : uint8_t
{
    HOUR = 0,
    MINUTE = 1,
    SECOND = 2
};

bool gCountdownEditing = false;
bool gCountdownConfigured = false;
CountdownEditField gCountdownField = CountdownEditField::HOUR;
TimeValue gEditCountdown = {0U, 0U, 0U};
TimeValue gSavedCountdown = {0U, 0U, 0U};

void printClockEditPrompt()
{
    Serial.print("CLOCK EDIT > ");
    switch (gClockField)
    {
        case ClockEditField::HOUR:
            Serial.println("HH");
            break;
        case ClockEditField::MINUTE:
            Serial.println("MM");
            break;
        case ClockEditField::SECOND:
            Serial.println("SS");
            break;
    }
    Serial.print("Current time: ");
    printTimeHhMmSs(gEditTime);
    Serial.println();
}

void beginClockEdit()
{
    DateTime currentTime;
    if (gTimeService.getDateTime(currentTime) == StatusCode::OK)
    {
        gEditTime = currentTime;
    }
    gClockEditing = true;
    gClockField = ClockEditField::HOUR;
    Serial.println("CLOCK setting started");
    printClockEditPrompt();
}

void advanceClockEditField()
{
    if (!gClockEditing)
    {
        return;
    }

    const uint8_t field = static_cast<uint8_t>(gClockField);
    gClockField = static_cast<ClockEditField>((field + 1U) % 3U);
    printClockEditPrompt();
}

void adjustClockField(int8_t delta)
{
    if (!gClockEditing)
    {
        return;
    }

    switch (gClockField)
    {
        case ClockEditField::HOUR:
            gEditTime.hour = static_cast<uint8_t>((gEditTime.hour + 24U + delta) % 24U);
            break;
        case ClockEditField::MINUTE:
            gEditTime.minute = static_cast<uint8_t>((gEditTime.minute + 60U + delta) % 60U);
            break;
        case ClockEditField::SECOND:
            gEditTime.second = static_cast<uint8_t>((gEditTime.second + 60U + delta) % 60U);
            break;
    }

    Serial.print("Edited time: ");
    printTimeHhMmSs(gEditTime);
    Serial.println();
}

void saveClockEdit()
{
    if (!gClockEditing)
    {
        return;
    }

    if (gTimeService.setDateTime(gEditTime) == StatusCode::OK)
    {
        gClockEditing = false;
        Serial.print("CLOCK saved: ");
        printTimeHhMmSs(gEditTime);
        Serial.println();
    }
    else
    {
        Serial.println("CLOCK save failed");
    }
}

void printCountdownEditPrompt()
{
    Serial.print("COUNTDOWN EDIT > ");
    switch (gCountdownField)
    {
        case CountdownEditField::HOUR:
            Serial.println("HH");
            break;
        case CountdownEditField::MINUTE:
            Serial.println("MM");
            break;
        case CountdownEditField::SECOND:
            Serial.println("SS");
            break;
    }
    Serial.print("Current countdown: ");
    printStopwatchValue(gEditCountdown);
    Serial.println();
}

void beginCountdownEdit()
{
    // Editing always starts from the saved configuration, rather than the
    // remaining value (which is zero after a completed countdown).
    gEditCountdown = gSavedCountdown;
    gCountdownEditing = true;
    gCountdownField = CountdownEditField::HOUR;
    Serial.println("COUNTDOWN setting started");
    printCountdownEditPrompt();
}

void advanceCountdownEditField()
{
    if (!gCountdownEditing)
    {
        return;
    }

    const uint8_t field = static_cast<uint8_t>(gCountdownField);
    gCountdownField = static_cast<CountdownEditField>((field + 1U) % 3U);
    printCountdownEditPrompt();
}

void adjustCountdownField(int8_t delta)
{
    if (!gCountdownEditing)
    {
        return;
    }

    switch (gCountdownField)
    {
        case CountdownEditField::HOUR:
            gEditCountdown.hour = static_cast<uint8_t>((gEditCountdown.hour + 100U + delta) % 100U);
            break;
        case CountdownEditField::MINUTE:
            gEditCountdown.minute = static_cast<uint8_t>((gEditCountdown.minute + 60U + delta) % 60U);
            break;
        case CountdownEditField::SECOND:
            gEditCountdown.second = static_cast<uint8_t>((gEditCountdown.second + 60U + delta) % 60U);
            break;
    }

    Serial.print("Edited countdown: ");
    printStopwatchValue(gEditCountdown);
    Serial.println();
}

void saveCountdownEdit()
{
    if (!gCountdownEditing)
    {
        return;
    }

    if (gTimeService.countdownSet(gEditCountdown) == StatusCode::OK)
    {
        gCountdownEditing = false;
        gCountdownConfigured = true;
        gSavedCountdown = gEditCountdown;
        Serial.print("COUNTDOWN saved: ");
        printStopwatchValue(gEditCountdown);
        Serial.println();
    }
    else
    {
        Serial.println("COUNTDOWN save failed");
    }
}

}  // namespace

ButtonDriver gButtonDriver;
DisplayDriver gDisplayDriver;
RtcDriver gRtcDriver;
Scheduler gScheduler;
EventSystem gEventSystem;
ot::ModeManager gModeManager;

bool gSystemEnabled = false;
bool gCountdownCompletionAlarmActive = false;
uint32_t gCountdownCompletionAlarmStartMs = 0U;
CountdownState gPreviousCountdownState = CountdownState::IDLE;

#if OPTIME_MILLIS_RTC_DIAGNOSTIC
enum class MillisRtcDiagnosticState : uint8_t
{
    MEASURING = 0U,
    COMPLETE,
    FAILED
};

MillisRtcDiagnosticState gMillisRtcDiagnosticState = MillisRtcDiagnosticState::MEASURING;
DateTime gMillisRtcDiagnosticStartRtc = {0U, 0U, 0U, 0U, 1U, 1U, 2000U};
uint32_t gMillisRtcDiagnosticStartMillis = 0U;
uint32_t gMillisRtcDiagnosticLastPollMillis = 0U;
constexpr uint32_t kMillisRtcDiagnosticPollIntervalMs = 250U;
#endif

#if OPTIME_TIMER1_DIAGNOSTIC
enum class Timer1DiagnosticState : uint8_t
{
    MEASURING = 0U,
    COMPLETE
};

Timer1DiagnosticState gTimer1DiagnosticState = Timer1DiagnosticState::MEASURING;
uint32_t gTimer1DiagnosticStartMs = 0U;
DateTime gTimer1DiagnosticStartRtc = {0U, 0U, 0U, 0U, 1U, 1U, 2000U};
bool gTimer1DiagnosticStartRtcValid = false;
constexpr uint32_t kTimer1DiagnosticDurationMs = OPTIME_TIMER1_DIAGNOSTIC_DURATION_MS;
#endif

// Stopwatch value tracking for precise serial print timing
TimeValue gLastPrintedStopwatch = {0U, 0U, 0U};
StopwatchState gLastPrintedStopwatchState = StopwatchState::STOPPED;

// Countdown value tracking for precise serial print timing
TimeValue gLastPrintedCountdown = {0U, 0U, 0U};
CountdownState gLastPrintedCountdownState = CountdownState::IDLE;

bool stopwatchValueChanged(const TimeValue& current, const StopwatchState currentState)
{
    if (current.hour != gLastPrintedStopwatch.hour || 
        current.minute != gLastPrintedStopwatch.minute || 
        current.second != gLastPrintedStopwatch.second ||
        currentState != gLastPrintedStopwatchState)
    {
        gLastPrintedStopwatch = current;
        gLastPrintedStopwatchState = currentState;
        return true;
    }
    return false;
}

bool countdownValueChanged(const TimeValue& current, const CountdownState currentState)
{
    if (current.hour != gLastPrintedCountdown.hour || 
        current.minute != gLastPrintedCountdown.minute || 
        current.second != gLastPrintedCountdown.second ||
        currentState != gLastPrintedCountdownState)
    {
        gLastPrintedCountdown = current;
        gLastPrintedCountdownState = currentState;
        return true;
    }
    return false;
}

void updateHardwareDisplay()
{
    if (!gSystemEnabled)
    {
        return;
    }

    uint8_t hour = 0U;
    uint8_t minute = 0U;
    uint8_t second = 0U;

    switch (gModeManager.currentMode())
    {
        case ot::AppMode::CLOCK:
            if (gClockEditing)
            {
                hour = gEditTime.hour;
                minute = gEditTime.minute;
                second = gEditTime.second;
            }
            else
            {
                DateTime time;
                if (gTimeService.getDateTime(time) == StatusCode::OK)
                {
                    hour = time.hour;
                    minute = time.minute;
                    second = time.second;
                }
            }
            break;

        case ot::AppMode::STOPWATCH:
        {
            TimeValue time;
            if (gTimeService.getStopwatch(time) == StatusCode::OK)
            {
                hour = time.hour;
                minute = time.minute;
                second = time.second;
            }
            break;
        }

        case ot::AppMode::COUNTDOWN:
        {
            TimeValue time;
            if (gCountdownEditing)
            {
                time = gEditCountdown;
            }
            else if (gTimeService.getCountdown(time) != StatusCode::OK)
            {
                time = {0U, 0U, 0U};
            }
            hour = time.hour;
            minute = time.minute;
            second = time.second;
            break;
        }

        default:
            break;
    }

    gDisplayDriver.setTime(hour, minute, second);

    // Blink the currently edited HH, MM, or SS pair. A value above 9 is
    // rendered by DisplayDriver as blank without changing the saved time.
    const bool blinkVisible = (millis() % (kEditFieldBlinkPeriodMs * 2U)) <
                              kEditFieldBlinkPeriodMs;
    if (!blinkVisible && (gClockEditing || gCountdownEditing))
    {
        uint8_t firstDigit = 0U;
        if (gClockEditing)
        {
            firstDigit = static_cast<uint8_t>(gClockField) * 2U;
        }
        else
        {
            firstDigit = static_cast<uint8_t>(gCountdownField) * 2U;
        }

        gDisplayDriver.setDigit(firstDigit, kDisplayBlankDigit);
        gDisplayDriver.setDigit(firstDigit + 1U, kDisplayBlankDigit);
    }
    // The colon output is the front-panel tick LED. It shares the Timer2
    // timebase used by TimeService, Stopwatch, and Countdown.
    gDisplayDriver.setColon((gScheduler.tick() % 1000U) < 500U);
    gDisplayDriver.swapBuffer();
}

void updatePowerLed()
{
    if (!gSystemEnabled)
    {
        digitalWrite(kPowerLedPin, HIGH);
        return;
    }

    const uint32_t nowMs = millis();
    bool ledOn = true;
    switch (gModeManager.currentMode())
    {
        case ot::AppMode::STOPWATCH:
            // 1 blink per second: 500 ms on, 500 ms off.
            ledOn = (nowMs % 1000U) < 500U;
            break;
        case ot::AppMode::COUNTDOWN:
            // 3 blinks per second: approximately 167 ms on, 167 ms off.
            ledOn = (nowMs % 333U) < 167U;
            break;
        case ot::AppMode::CLOCK:
        default:
            ledOn = true;
            break;
    }

    // Power LED hardware is active-low.
    digitalWrite(kPowerLedPin, ledOn ? LOW : HIGH);
}

void resetStopwatchOnModeExit()
{
    if (gModeManager.currentMode() != ot::AppMode::STOPWATCH)
    {
        return;
    }

    const StopwatchState state = gTimeService.stopwatchState();
    if (state == StopwatchState::RUNNING || state == StopwatchState::PAUSED)
    {
        (void)gTimeService.stopwatchReset();
        Serial.println("STOPWATCH reset on mode exit");
    }
}

void resetCountdownOnModeExit()
{
    if (gModeManager.currentMode() != ot::AppMode::COUNTDOWN)
    {
        return;
    }

    (void)gTimeService.countdownReset();
    gCountdownEditing = false;
    gCountdownConfigured = false;
    gSavedCountdown = {0U, 0U, 0U};
    gCountdownCompletionAlarmActive = false;
    Serial.println("COUNTDOWN reset on mode exit");
}

#if OPTIME_TIMER1_DIAGNOSTIC
uint32_t timerDiagnosticSecondsOfDay(const DateTime& time)
{
    return (static_cast<uint32_t>(time.hour) * 3600UL) +
           (static_cast<uint32_t>(time.minute) * 60UL) +
           static_cast<uint32_t>(time.second);
}

uint32_t timerDiagnosticRtcElapsedSeconds(const DateTime& start, const DateTime& end)
{
    const uint32_t startSeconds = timerDiagnosticSecondsOfDay(start);
    const uint32_t endSeconds = timerDiagnosticSecondsOfDay(end);
    if (endSeconds >= startSeconds)
    {
        return endSeconds - startSeconds;
    }

    return (86400UL - startSeconds) + endSeconds;
}

void beginTimer1Diagnostic()
{
    gSystemEnabled = true;
    (void)gModeManager.setMode(ot::AppMode::STOPWATCH);
    (void)gTimeService.stopwatchReset();
    gTimer1DiagnosticStartRtcValid = (gRtcDriver.read(gTimer1DiagnosticStartRtc) == StatusCode::OK);
    gTimer1DiagnosticStartMs = ::millis();
    (void)gTimeService.stopwatchStart();
#if OPTIME_TIMER1_DIAGNOSTIC_TIMER1_OFF
    gDisplayDriver.disableRefresh();
#endif
    gTimer1DiagnosticState = Timer1DiagnosticState::MEASURING;

    Serial.println("TIMER1 DIAGNOSTIC START");
    Serial.print("F_CPU = ");
    Serial.println(F_CPU);
    Serial.print("RTC start = ");
    printTimeHhMmSs(gTimer1DiagnosticStartRtc);
    Serial.println();
#if OPTIME_TIMER1_DIAGNOSTIC_TIMER1_OFF
    Serial.println("Timer1 = OFF");
#else
    Serial.println("Timer1 = ON");
#endif
}

void processTimer1Diagnostic()
{
    gScheduler.run();
    gTimeService.update();

    if ((uint32_t)(::millis() - gTimer1DiagnosticStartMs) < kTimer1DiagnosticDurationMs)
    {
        return;
    }

    const uint32_t endMillis = ::millis();
    DateTime endRtc = {0U, 0U, 0U, 0U, 1U, 1U, 2000U};
    const bool endRtcValid = (gRtcDriver.read(endRtc) == StatusCode::OK);
    const uint32_t millisElapsed = endMillis - gTimer1DiagnosticStartMs;
    const uint32_t stopwatchElapsed = gTimeService.stopwatchElapsedMilliseconds();
    const uint32_t rtcElapsedSeconds =
        (gTimer1DiagnosticStartRtcValid && endRtcValid)
            ? timerDiagnosticRtcElapsedSeconds(gTimer1DiagnosticStartRtc, endRtc)
            : 0U;
    const uint32_t rtcElapsedMs = rtcElapsedSeconds * 1000UL;
    const int32_t millisDifference = static_cast<int32_t>(millisElapsed - rtcElapsedMs);
    const int32_t stopwatchDifference = static_cast<int32_t>(stopwatchElapsed - rtcElapsedMs);
    const uint32_t absoluteMillisDifference =
        millisDifference < 0 ? static_cast<uint32_t>(-millisDifference)
                             : static_cast<uint32_t>(millisDifference);
    const uint32_t errorFraction =
        rtcElapsedMs == 0U ? 0U : (absoluteMillisDifference * 1000000UL) / rtcElapsedMs;

#if OPTIME_TIMER1_DIAGNOSTIC_TIMER1_OFF
    gDisplayDriver.enableRefresh();
#endif
    gTimer1DiagnosticState = Timer1DiagnosticState::COMPLETE;
    updateHardwareDisplay();

    Serial.println("DIAGNOSTIC");
    Serial.print("RTC start: ");
    printTimeHhMmSs(gTimer1DiagnosticStartRtc);
    Serial.println();
    Serial.print("RTC end: ");
    printTimeHhMmSs(endRtc);
    Serial.println();
    Serial.println("RTC resolution: +/-1 second");
    Serial.print("Expected ms: ");
    Serial.println(kTimer1DiagnosticDurationMs);
    Serial.print("millis ms: ");
    Serial.println(millisElapsed);
    Serial.print("RTC seconds: ");
    Serial.println(rtcElapsedSeconds);
    Serial.print("millis - RTC ms: ");
    Serial.println(millisDifference);
    Serial.print("millis error: ");
    Serial.print(errorFraction / 10000U);
    Serial.print(".");
    const uint32_t errorFractionRemainder = errorFraction % 10000U;
    if (errorFractionRemainder < 1000U)
    {
        Serial.print("0");
    }
    if (errorFractionRemainder < 100U)
    {
        Serial.print("0");
    }
    if (errorFractionRemainder < 10U)
    {
        Serial.print("0");
    }
    Serial.println(errorFractionRemainder);
    Serial.print("stopwatch ms: ");
    Serial.println(stopwatchElapsed);
    Serial.print("stopwatch - RTC ms: ");
    Serial.println(stopwatchDifference);
    Serial.print("stopwatch display seconds: ");
    Serial.println(stopwatchElapsed / 1000UL);
}
#endif

#if OPTIME_MILLIS_RTC_DIAGNOSTIC
uint32_t diagnosticDaysInMonth(const uint16_t year, const uint8_t month)
{
    static const uint8_t daysInMonth[] = {31U, 28U, 31U, 30U, 31U, 30U,
                                           31U, 31U, 30U, 31U, 30U, 31U};
    if (month == 2U && ((year % 4U) == 0U))
    {
        return 29U;
    }
    return daysInMonth[month - 1U];
}

uint32_t diagnosticDateToDays(const DateTime& time)
{
    uint32_t days = 0U;
    for (uint16_t year = 2000U; year < time.year; ++year)
    {
        days += ((year % 4U) == 0U) ? 366U : 365U;
    }
    for (uint8_t month = 1U; month < time.month; ++month)
    {
        days += diagnosticDaysInMonth(time.year, month);
    }
    return days + static_cast<uint32_t>(time.date - 1U);
}

uint32_t diagnosticRtcElapsedSeconds(const DateTime& start, const DateTime& end)
{
    const uint32_t startDay = diagnosticDateToDays(start);
    const uint32_t endDay = diagnosticDateToDays(end);
    const uint32_t startSeconds = (startDay * 86400UL) +
                                  (static_cast<uint32_t>(start.hour) * 3600UL) +
                                  (static_cast<uint32_t>(start.minute) * 60UL) +
                                  start.second;
    const uint32_t endSeconds = (endDay * 86400UL) +
                                (static_cast<uint32_t>(end.hour) * 3600UL) +
                                (static_cast<uint32_t>(end.minute) * 60UL) +
                                end.second;
    return endSeconds - startSeconds;
}

void beginMillisRtcDiagnostic()
{
    gSystemEnabled = true;
    (void)gModeManager.setMode(ot::AppMode::STOPWATCH);
    (void)gTimeService.stopwatchReset();

    if (gRtcDriver.read(gMillisRtcDiagnosticStartRtc) != StatusCode::OK)
    {
        gMillisRtcDiagnosticState = MillisRtcDiagnosticState::FAILED;
        return;
    }

    gMillisRtcDiagnosticStartMillis = ::millis();
    (void)gTimeService.stopwatchStart();
#if OPTIME_MILLIS_RTC_DIAGNOSTIC_TIMER1_OFF
    gDisplayDriver.disableRefresh();
#endif
    gMillisRtcDiagnosticLastPollMillis = gMillisRtcDiagnosticStartMillis;
    gMillisRtcDiagnosticState = MillisRtcDiagnosticState::MEASURING;

    Serial.println("ATmega328P clock diagnostic");
    Serial.print("F_CPU: ");
    Serial.println(F_CPU);
    Serial.print("Clock source: ");
    Serial.println("UNKNOWN - verify hardware");
    Serial.print("RTC start: ");
    printTimeHhMmSs(gMillisRtcDiagnosticStartRtc);
    Serial.println();
#if OPTIME_MILLIS_RTC_DIAGNOSTIC_TIMER1_OFF
    Serial.println("Timer1: DISABLED");
#else
    Serial.println("Timer1: ENABLED");
#endif
}

void processMillisRtcDiagnostic()
{
    gScheduler.run();
    gTimeService.update();

    const uint32_t nowMillis = ::millis();
    if ((uint32_t)(nowMillis - gMillisRtcDiagnosticLastPollMillis) <
        kMillisRtcDiagnosticPollIntervalMs)
    {
        return;
    }
    gMillisRtcDiagnosticLastPollMillis = nowMillis;

    DateTime endRtc = {0U, 0U, 0U, 0U, 1U, 1U, 2000U};
    if (gRtcDriver.read(endRtc) != StatusCode::OK)
    {
        return;
    }

    const uint32_t rtcElapsedSeconds =
        diagnosticRtcElapsedSeconds(gMillisRtcDiagnosticStartRtc, endRtc);
    if (rtcElapsedSeconds < OPTIME_MILLIS_RTC_DIAGNOSTIC_DURATION_SECONDS)
    {
        return;
    }

    const uint32_t endMillis = ::millis();
    const uint32_t millisElapsed = endMillis - gMillisRtcDiagnosticStartMillis;
    const uint32_t stopwatchElapsed = gTimeService.stopwatchElapsedMilliseconds();
    const uint32_t rtcElapsedMs = rtcElapsedSeconds * 1000UL;
    const int32_t millisDifference = static_cast<int32_t>(millisElapsed - rtcElapsedMs);
    const int32_t stopwatchDifference =
        static_cast<int32_t>(stopwatchElapsed - rtcElapsedMs);

#if OPTIME_MILLIS_RTC_DIAGNOSTIC_TIMER1_OFF
    gDisplayDriver.enableRefresh();
#endif
    gMillisRtcDiagnosticState = MillisRtcDiagnosticState::COMPLETE;
    updateHardwareDisplay();

    Serial.println("=== MILLIS vs RTC ===");
    Serial.print("RTC start: ");
    printTimeHhMmSs(gMillisRtcDiagnosticStartRtc);
    Serial.println();
    Serial.print("RTC end: ");
    printTimeHhMmSs(endRtc);
    Serial.println();
    Serial.println("RTC resolution: +/-1 second");
    Serial.print("RTC elapsed: ");
    Serial.print(rtcElapsedSeconds);
    Serial.println(" s");
    Serial.print("millis elapsed: ");
    Serial.print(millisElapsed);
    Serial.println(" ms");
    Serial.print("difference: ");
    Serial.print(millisDifference);
    Serial.println(" ms");
    Serial.print("stopwatch elapsed: ");
    Serial.print(stopwatchElapsed);
    Serial.println(" ms");
    Serial.print("stopwatch - RTC: ");
    Serial.print(stopwatchDifference);
    Serial.println(" ms");
    Serial.print("stopwatch display seconds: ");
    Serial.println(stopwatchElapsed / 1000UL);
}
#endif

void setSystemOff()
{
    gSystemEnabled = false;
    gCountdownCompletionAlarmActive = false;
    gPreviousCountdownState = CountdownState::IDLE;
    gDisplayDriver.clear();
    gDisplayDriver.swapBuffer();
    digitalWrite(kPowerLedPin, HIGH);
    digitalWrite(kBuzzerPin, HIGH);
    Serial.println("BOOT: system OFF");
}

void setSystemOn()
{
    gSystemEnabled = true;
    digitalWrite(kPowerLedPin, LOW);
    digitalWrite(kBuzzerPin, HIGH);

    (void)gModeManager.setMode(ot::AppMode::CLOCK);
    Serial.println("BOOT: system ON");
    Serial.print("Default mode: ");
    Serial.println(modeName(gModeManager.currentMode()));
}

void setup()
{
    Serial.begin(9600);
    delay(200U);

    pinMode(kPowerLedPin, OUTPUT);
    pinMode(kBuzzerPin, OUTPUT);
    digitalWrite(kPowerLedPin, LOW);
    digitalWrite(kBuzzerPin, LOW);

    Serial.println("=== Operation Timer firmware debug ===");
    Serial.println("Boot state: OFF");
    Serial.println("Display hardware enabled");

    (void)gButtonDriver.begin();
    if (gDisplayDriver.begin() == StatusCode::OK)
    {
        gDisplayDriver.enableRefresh();
    }
    else
    {
        Serial.println("Display initialization failed");
    }
    (void)gRtcDriver.begin();
    (void)gScheduler.begin();
    (void)gEventSystem.begin();
    (void)gModeManager.begin(&gEventSystem);
    (void)gTimeService.begin(&gRtcDriver, &gScheduler, &gEventSystem);

    setSystemOff();
#if OPTIME_MILLIS_RTC_DIAGNOSTIC
    beginMillisRtcDiagnostic();
#elif OPTIME_TIMER1_DIAGNOSTIC
    beginTimer1Diagnostic();
#endif
    Serial.println("Waiting for POWER button to activate system...");
}

void loop()
{
#if OPTIME_MILLIS_RTC_DIAGNOSTIC
    if (gMillisRtcDiagnosticState == MillisRtcDiagnosticState::MEASURING)
    {
        processMillisRtcDiagnostic();
        return;
    }
#elif OPTIME_TIMER1_DIAGNOSTIC
    if (gTimer1DiagnosticState == Timer1DiagnosticState::MEASURING)
    {
        processTimer1Diagnostic();
        return;
    }
#endif
    gButtonDriver.update();

    ButtonEvent buttonEvent;
    while (gButtonDriver.getEvent(buttonEvent))
    {
        Serial.print("BUTTON | id=");
        Serial.print(buttonName(buttonEvent.id));
        Serial.print(" | event=");
        Serial.print(buttonEventName(buttonEvent.type));
        Serial.println();

        if (!gSystemEnabled)
        {
            if (buttonEvent.id == BUTTON_POWER && buttonEvent.type == BUTTON_SHORT_PRESS)
            {
                setSystemOn();
            }
            continue;
        }

        if (buttonEvent.id == BUTTON_POWER && buttonEvent.type == BUTTON_SHORT_PRESS)
        {
            setSystemOff();
            Serial.println("POWER OFF");
            continue;
        }

        if (gModeManager.currentMode() == ot::AppMode::CLOCK)
        {
            if (!gClockEditing)
            {
                if (buttonEvent.id == BUTTON_NEXT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    (void)gModeManager.nextMode();
                    Serial.print("MODE changed: ");
                    Serial.println(modeName(gModeManager.currentMode()));
                    continue;
                }

                if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    beginClockEdit();
                    continue;
                }
            }
            else
            {
                if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    saveClockEdit();
                    continue;
                }

                if (buttonEvent.id == BUTTON_NEXT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    advanceClockEditField();
                    continue;
                }

                if (buttonEvent.id == BUTTON_UP &&
                    (buttonEvent.type == BUTTON_SHORT_PRESS || buttonEvent.type == BUTTON_REPEAT))
                {
                    adjustClockField(1);
                    continue;
                }

                if (buttonEvent.id == BUTTON_DOWN &&
                    (buttonEvent.type == BUTTON_SHORT_PRESS || buttonEvent.type == BUTTON_REPEAT))
                {
                    adjustClockField(-1);
                    continue;
                }
            }
        }

        if (gModeManager.currentMode() == ot::AppMode::STOPWATCH)
        {
            if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_HOLD)
            {
                (void)gTimeService.stopwatchReset();
                Serial.println("STOPWATCH reset");
                continue;
            }

            if (buttonEvent.id == BUTTON_NEXT && buttonEvent.type == BUTTON_SHORT_PRESS)
            {
                resetStopwatchOnModeExit();
                (void)gModeManager.nextMode();
                Serial.print("MODE changed: ");
                Serial.println(modeName(gModeManager.currentMode()));
                continue;
            }

            if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_SHORT_PRESS)
            {
                const StopwatchState state = gTimeService.stopwatchState();
                if (state == StopwatchState::RUNNING)
                {
                    (void)gTimeService.stopwatchPause();
                    Serial.println("STOPWATCH paused");
                }
                else
                {
                    (void)gTimeService.stopwatchStart();
                    Serial.println("STOPWATCH started");
                }
                continue;
            }

            if (buttonEvent.id == BUTTON_UP)
            {
                Serial.println("UP ignored in stopwatch mode");
                continue;
            }
        }

        if (gModeManager.currentMode() == ot::AppMode::COUNTDOWN)
        {
            if (!gCountdownEditing)
            {
                if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_HOLD)
                {
                    (void)gTimeService.countdownReset();
                    gCountdownConfigured = false;
                    gSavedCountdown = {0U, 0U, 0U};
                    Serial.println("COUNTDOWN reset");
                    continue;
                }

                if (buttonEvent.id == BUTTON_NEXT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    resetCountdownOnModeExit();
                    (void)gModeManager.setMode(ot::AppMode::CLOCK);
                    Serial.print("MODE changed: ");
                    Serial.println(modeName(gModeManager.currentMode()));
                    continue;
                }

                if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    const CountdownState state = gTimeService.countdownState();
                    if (!gCountdownConfigured)
                    {
                        beginCountdownEdit();
                        continue;
                    }

                    if (state == CountdownState::RUNNING)
                    {
                        (void)gTimeService.countdownPause();
                        Serial.println("COUNTDOWN paused");
                    }
                    else
                    {
                        (void)gTimeService.countdownStart();
                        Serial.println(state == CountdownState::PAUSED ? "COUNTDOWN resumed"
                                                                        : "COUNTDOWN started");
                    }
                    continue;
                }

                const CountdownState state = gTimeService.countdownState();
                if ((state == CountdownState::PAUSED || state == CountdownState::COMPLETED) &&
                    (buttonEvent.id == BUTTON_UP || buttonEvent.id == BUTTON_DOWN) &&
                    buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    beginCountdownEdit();
                    continue;
                }
            }
            else
            {
                if (buttonEvent.id == BUTTON_SELECT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    saveCountdownEdit();
                    continue;
                }

                if (buttonEvent.id == BUTTON_NEXT && buttonEvent.type == BUTTON_SHORT_PRESS)
                {
                    advanceCountdownEditField();
                    continue;
                }

                if (buttonEvent.id == BUTTON_UP &&
                    (buttonEvent.type == BUTTON_SHORT_PRESS || buttonEvent.type == BUTTON_REPEAT))
                {
                    adjustCountdownField(1);
                    continue;
                }

                if (buttonEvent.id == BUTTON_DOWN &&
                    (buttonEvent.type == BUTTON_SHORT_PRESS || buttonEvent.type == BUTTON_REPEAT))
                {
                    adjustCountdownField(-1);
                    continue;
                }
            }

            if (!gCountdownEditing && buttonEvent.id == BUTTON_UP)
            {
                Serial.println("UP ignored in countdown mode");
                continue;
            }
        }

        if (buttonEvent.type == BUTTON_HOLD &&
            buttonEvent.id != BUTTON_UP && buttonEvent.id != BUTTON_DOWN)
        {
            beepOnce(120U);
            Serial.println("HOLD detected");
        }

        if (buttonEvent.type == BUTTON_REPEAT &&
            buttonEvent.id != BUTTON_UP && buttonEvent.id != BUTTON_DOWN)
        {
            beepOnce(40U);
            Serial.println("REPEAT detected");
        }
    }

    if (!gSystemEnabled)
    {
        digitalWrite(kPowerLedPin, HIGH);
        digitalWrite(kBuzzerPin, HIGH);
        return;
    }

    // UP/DOWN are adjustment controls and must remain silent for short,
    // hold, and repeat input. Other pressed buttons retain the feedback tone.
    digitalWrite(kBuzzerPin, gButtonDriver.anyNonAdjustmentButtonPressed() ? LOW : HIGH);

    gScheduler.run();
    gTimeService.update();
    updateHardwareDisplay();
    updatePowerLed();

    // Play one non-blocking, three-second active-low tone when the countdown
    // transitions into COMPLETED.
    const CountdownState countdownState = gTimeService.countdownState();
    const uint32_t nowMs = millis();
    if (countdownState == CountdownState::COMPLETED &&
        gPreviousCountdownState != CountdownState::COMPLETED)
    {
        gCountdownCompletionAlarmActive = true;
        gCountdownCompletionAlarmStartMs = nowMs;
    }
    else if (gCountdownCompletionAlarmActive &&
             (uint32_t)(nowMs - gCountdownCompletionAlarmStartMs) >= kCountdownCompletionToneMs)
    {
        gCountdownCompletionAlarmActive = false;
    }
    else if (countdownState != CountdownState::COMPLETED)
    {
        gCountdownCompletionAlarmActive = false;
    }
    gPreviousCountdownState = countdownState;
    digitalWrite(kBuzzerPin,
                 (gCountdownCompletionAlarmActive || gButtonDriver.anyNonAdjustmentButtonPressed()) ? LOW : HIGH);

    // Print active timer values on change for precise second-by-second output.
    if (gModeManager.currentMode() == ot::AppMode::STOPWATCH)
    {
        TimeValue sw;
        if (gTimeService.getStopwatch(sw) == StatusCode::OK)
        {
            if (stopwatchValueChanged(sw, gTimeService.stopwatchState()))
            {
                Serial.print("MODE=");
                Serial.print(modeName(gModeManager.currentMode()));
                Serial.print(" | STOPWATCH=");
                printStopwatchValue(sw);
                Serial.print(" | STATE=");
                Serial.println(stopwatchStateName(gTimeService.stopwatchState()));
            }
        }
    }
    else if (gModeManager.currentMode() == ot::AppMode::COUNTDOWN)
    {
        TimeValue countdown;
        if (gTimeService.getCountdown(countdown) == StatusCode::OK &&
            countdownValueChanged(countdown, gTimeService.countdownState()))
        {
            Serial.print("MODE=");
            Serial.print(modeName(gModeManager.currentMode()));
            Serial.print(" | COUNTDOWN=");
            printStopwatchValue(countdown);
            Serial.print(" | STATE=");
            Serial.println(countdownStateName(gTimeService.countdownState()));
        }
    }
    else
    {
        // For other modes, print clock every 1 second
        static uint32_t lastStatusMs = 0U;
        const uint32_t now = millis();
        if ((now - lastStatusMs) >= 1000U)
        {
            lastStatusMs = now;

            DateTime nowRtc;
            if (gTimeService.getDateTime(nowRtc) == StatusCode::OK)
            {
                Serial.print("MODE=");
                Serial.print(modeName(gModeManager.currentMode()));
                Serial.print(" | RTC=");
                printTimeHhMmSs(nowRtc);
                Serial.print(" | DATE=");
                printDateTime(nowRtc);
                Serial.print(" | RTC_VALID=");
                Serial.println(gTimeService.isRtcValid() ? "YES" : "NO");
            }
            else
            {
                Serial.println("MODE=CLOCK | RTC read failed");
            }
        }
    }
}

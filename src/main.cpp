#include "Arduino.h"

#include "app/ModeManager.h"
#include "bsp/BoardConfig.h"
#include "core/EventSystem.h"
#include "drivers/ButtonDriver.h"
#include "drivers/DisplayDriver.h"
#include "drivers/RtcDriver.h"
#include "scheduler/Scheduler.h"
#include "services/TimeService.h"

namespace
{
const uint8_t kPowerLedPin = Board::Pin::POWER_LED;
const uint8_t kBuzzerPin = Board::Pin::BUZZER;
constexpr uint32_t kCountdownCompletionToneMs = 3000U;
constexpr uint8_t kDisplayBlankDigit = 0xFFU;
constexpr uint32_t kEditFieldBlinkPeriodMs = 500U;
TimeService gTimeService;

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
        gDisplayDriver.setBrightness(75);
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
    Serial.println("Waiting for POWER button to activate system...");
}

void loop()
{
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

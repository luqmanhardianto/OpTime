#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "core/EventSystem.h"

namespace ot
{
enum class AppMode : uint8_t
{
    CLOCK = 0,
    STOPWATCH,
    COUNTDOWN,
    FACTORY,
    DIAGNOSTIC
};

using SystemMode = AppMode;

struct ModeChangeRequest
{
    AppMode targetMode;
    EventSource source;
};

class ModeManager
{
public:
    ModeManager();

    StatusCode begin();
    StatusCode begin(EventSystem* eventSystem);

    void update();

    StatusCode setMode(AppMode mode);
    StatusCode setMode(const ModeChangeRequest& request);
    StatusCode nextMode();

    AppMode currentMode() const;
    AppMode previousMode() const;
    bool isMode(AppMode mode) const;

private:
    static bool isNormalMode(AppMode mode);
    static bool isValidTransition(AppMode current, AppMode target);
    static AppMode nextModeFor(AppMode current);
    static uint16_t encodeTransition(AppMode previous, AppMode current);

    void publishModeChangeEvent(AppMode previous, AppMode current, EventSource source);
    void handleEvent(const Event& event);

    EventSystem* eventSystem_;
    AppMode currentMode_;
    AppMode previousMode_;
};
}  // namespace ot

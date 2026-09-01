#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "hal/TimerHal.h"

using TaskCallback = void (*)();

struct Task
{
    TaskCallback callback;
    uint16_t periodMs;
    uint32_t nextRun;
    bool enabled;
};

class Scheduler
{
public:
    StatusCode begin();
    StatusCode addTask(Task& task);
    void run();
    uint32_t tick() const;

private:
    static constexpr uint8_t kMaxTasks = 16U;

    Task tasks_[kMaxTasks];
    uint8_t taskCount_;
    volatile uint32_t systemTick_;
    TimerHal timerHal_;

    static void onSystemTick();
};

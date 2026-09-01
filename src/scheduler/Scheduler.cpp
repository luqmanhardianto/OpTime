#include "scheduler/Scheduler.h"

#include <Arduino.h>

namespace
{
Scheduler* gSchedulerInstance = nullptr;
}

void Scheduler::onSystemTick()
{
    if (gSchedulerInstance != nullptr)
    {
        gSchedulerInstance->systemTick_++;
    }
}

StatusCode Scheduler::begin()
{
    taskCount_ = 0U;
    systemTick_ = 0U;
    gSchedulerInstance = this;

    const StatusCode status = timerHal_.begin();
    if (status != StatusCode::OK)
    {
        return status;
    }

    timerHal_.attachTickCallback(&Scheduler::onSystemTick);
    return StatusCode::OK;
}

StatusCode Scheduler::addTask(Task& task)
{
    if (taskCount_ >= kMaxTasks)
    {
        return StatusCode::NO_RESOURCE;
    }

    tasks_[taskCount_] = task;
    taskCount_++;
    return StatusCode::OK;
}

void Scheduler::run()
{
    const uint32_t now = systemTick_;

    for (uint8_t i = 0; i < taskCount_; ++i)
    {
        Task& task = tasks_[i];
        if (!task.enabled)
        {
            continue;
        }

        if ((now - task.nextRun) < task.periodMs)
        {
            continue;
        }

        task.nextRun = now + task.periodMs;
        if (task.callback != nullptr)
        {
            task.callback();
        }
    }
}

uint32_t Scheduler::tick() const
{
    return systemTick_;
}

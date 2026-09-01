#include <Arduino.h>

#include "scheduler/Scheduler.h"

namespace
{
volatile uint8_t counter = 0U;
void testTask()
{
    counter++;
}
}

void setup()
{
    Serial.begin(9600);

    Scheduler scheduler;
    Task task = {testTask, 10U, 0U, true};

    if (scheduler.begin() != StatusCode::OK)
    {
        Serial.println("Scheduler init failed");
        return;
    }

    if (scheduler.addTask(task) != StatusCode::OK)
    {
        Serial.println("Task add failed");
        return;
    }

    Serial.println("Scheduler tests completed");
}

void loop()
{
}

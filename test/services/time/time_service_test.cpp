#include <Arduino.h>

#include "services/TimeService.h"

namespace
{
void validateClockRead()
{
    TimeService service;
    RtcDriver rtc;
    Scheduler scheduler;
    EventSystem eventSystem;

    service.begin(&rtc, &scheduler, &eventSystem);
    DateTime time = {0U, 0U, 0U, 0U, 1U, 1U, 2024U};
    const StatusCode status = service.setDateTime(time);
    if (status != StatusCode::OK)
    {
        Serial.println("setDateTime failed");
    }
}
}

void setup()
{
    Serial.begin(9600);
    validateClockRead();
    Serial.println("TimeService smoke test complete");
}

void loop()
{
}

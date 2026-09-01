#include <Arduino.h>

#include "drivers/RtcDriver.h"

void setup()
{
    Serial.begin(9600);

    RtcDriver rtc;
    const StatusCode status = rtc.begin();
    if (status != StatusCode::OK && status != StatusCode::NOT_READY)
    {
        Serial.println("RTC init failed");
        return;
    }

    DateTime time = {0U, 0U, 0U, 0U, 0U, 0U, 2000U};
    const StatusCode writeStatus = rtc.set(time);
    if (writeStatus != StatusCode::OK && writeStatus != StatusCode::INVALID_PARAMETER)
    {
        Serial.println("RTC write failed");
        return;
    }

    Serial.println("RTC driver tests completed");
}

void loop()
{
}

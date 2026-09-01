#include <Arduino.h>

#include "hal/TimerHal.h"

namespace
{
volatile uint32_t callbackCount = 0U;
void testCallback()
{
    callbackCount++;
}
}

void setup()
{
    Serial.begin(9600);

    TimerHal timerHal;
    timerHal.begin();
    timerHal.attachTickCallback(testCallback);

    delay(10);

    if (timerHal.millis() < 10U)
    {
        Serial.println("Timer init test failed");
    }

    Serial.println("Timer HAL tests completed");
}

void loop()
{
}

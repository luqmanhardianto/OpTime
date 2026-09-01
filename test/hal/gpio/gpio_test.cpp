#include <Arduino.h>

#include "hal/GpioHal.h"

void setup()
{
    Serial.begin(9600);

    GpioHal gpioHal;
    const StatusCode status = gpioHal.begin();

    if (status != StatusCode::OK)
    {
        Serial.println("GPIO init test failed");
    }

    gpioHal.writeActiveLow(PIN_POWER_LED, true);
    gpioHal.writeActiveLow(PIN_POWER_LED, false);

    Serial.println("GPIO HAL tests completed");
}

void loop()
{
}

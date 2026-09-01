#include <Arduino.h>

#include "drivers/ButtonDriver.h"

void setup()
{
    Serial.begin(9600);

    ButtonDriver driver;
    if (driver.begin() != StatusCode::OK)
    {
        Serial.println("Button driver init failed");
        return;
    }

    Serial.println("Button driver tests completed");
}

void loop()
{
}

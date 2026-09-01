#include <Arduino.h>

#include "drivers/DisplayDriver.h"

namespace
{
DisplayDriver driver;
}

void setup()
{
    Serial.begin(9600);

    if (driver.begin() != StatusCode::OK)
    {
        Serial.println("Display init failed");
        return;
    }

    driver.clear();
    driver.setDigit(0, 1U);
    driver.setDigit(1, 2U);
    driver.setDigit(2, 3U);
    driver.setDigit(3, 4U);
    driver.setDigit(4, 5U);
    driver.setDigit(5, 6U);
    driver.swapBuffer();
    driver.setColon(true);

    Serial.println("Display driver tests completed");
}

void loop()
{
}

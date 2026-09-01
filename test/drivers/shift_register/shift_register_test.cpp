#include <Arduino.h>

#include "drivers/ShiftRegisterDriver.h"

void setup()
{
    Serial.begin(9600);

    ShiftRegisterDriver driver;
    const StatusCode status = driver.begin();
    if (status != StatusCode::OK)
    {
        Serial.println("Shift register init failed");
        return;
    }

    driver.shiftOut(0xAA, 0x55);
    driver.latch();
    driver.setOutputEnable(true);
    driver.setOutputEnable(false);

    Serial.println("Shift register tests completed");
}

void loop()
{
}

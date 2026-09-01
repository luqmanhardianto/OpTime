#include <Arduino.h>

#include "hal/I2cHal.h"

void setup()
{
    Serial.begin(9600);

    I2cHal i2cHal;
    const StatusCode status = i2cHal.begin();
    if (status != StatusCode::OK)
    {
        Serial.println("I2C init failed");
        return;
    }

    uint8_t value = 0U;
    const StatusCode regStatus = i2cHal.readRegister(0x68, 0x0E, value);
    if (regStatus == StatusCode::OK)
    {
        Serial.println("DS3231 register read OK");
    }
    else
    {
        Serial.println("DS3231 register read failed");
    }
}

void loop()
{
}

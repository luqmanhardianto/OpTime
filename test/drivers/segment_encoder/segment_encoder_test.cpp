#include <Arduino.h>

#include "drivers/SegmentEncoder.h"

void setup()
{
    Serial.begin(9600);

    if (SegmentEncoder::encodeDigit(8) != 0xFE)
    {
        Serial.println("Digit 8 test failed");
    }

    if (SegmentEncoder::encodeDigit(10) != SegmentEncoder::blank())
    {
        Serial.println("Invalid digit test failed");
    }

    if (SegmentEncoder::encodeChar(SEG_DASH) != 0x08)
    {
        Serial.println("Dash test failed");
    }

    Serial.println("Segment encoder tests completed");
}

void loop()
{
}

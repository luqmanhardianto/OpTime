#pragma once

#include <stdint.h>

enum SegmentChar
{
    SEG_BLANK = 0,
    SEG_DASH,
    SEG_ERROR
};

class SegmentEncoder
{
public:
    static uint8_t encodeDigit(uint8_t digit);
    static uint8_t encodeChar(SegmentChar character);
    static uint8_t blank();
};

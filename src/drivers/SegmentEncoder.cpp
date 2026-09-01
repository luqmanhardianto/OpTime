#include "drivers/SegmentEncoder.h"

#include <avr/pgmspace.h>

namespace
{
// Actual hardware pin mapping on the segment 74HC595:
// QA NC
// QB B
// QC A
// QD F
// QE G
// QF C
// QG D
// QH E
// shiftOut() sends LSB first, so byte bit 7 reaches QA and bit 0 reaches QH.
// Segment output values are binary plus decimal for direct hardware tracing.
constexpr uint8_t kBitE = 0b00000001U;  // QH, E,  1
constexpr uint8_t kBitD = 0b00000010U;  // QG, D,  2
constexpr uint8_t kBitC = 0b00000100U;  // QF, C,  4
constexpr uint8_t kBitG = 0b00001000U;  // QE, G,  8
constexpr uint8_t kBitF = 0b00010000U;  // QD, F, 16
constexpr uint8_t kBitA = 0b00100000U;  // QC, A, 32
constexpr uint8_t kBitB = 0b01000000U;  // QB, B, 64

const uint8_t kDigitPattern[] PROGMEM = {
    static_cast<uint8_t>(kBitA | kBitB | kBitC | kBitD | kBitE | kBitF),
    static_cast<uint8_t>(kBitB | kBitC),
    static_cast<uint8_t>(kBitA | kBitB | kBitD | kBitE | kBitG),
    static_cast<uint8_t>(kBitA | kBitB | kBitC | kBitD | kBitG),
    static_cast<uint8_t>(kBitB | kBitC | kBitF | kBitG),
    static_cast<uint8_t>(kBitA | kBitC | kBitD | kBitF | kBitG),
    static_cast<uint8_t>(kBitA | kBitC | kBitD | kBitE | kBitF | kBitG),
    static_cast<uint8_t>(kBitA | kBitB | kBitC),
    static_cast<uint8_t>(kBitA | kBitB | kBitC | kBitD | kBitE | kBitF | kBitG),
    static_cast<uint8_t>(kBitA | kBitB | kBitC | kBitD | kBitF | kBitG)
};

constexpr uint8_t kBlankPattern = 0x00U;
constexpr uint8_t kDashPattern = kBitG;
constexpr uint8_t kErrorPattern = kBitA | kBitD | kBitE | kBitF | kBitG;
}

uint8_t SegmentEncoder::encodeDigit(uint8_t digit)
{
    if (digit > 9U)
    {
        return blank();
    }

    return static_cast<uint8_t>(pgm_read_byte(&kDigitPattern[digit]));
}

uint8_t SegmentEncoder::encodeChar(SegmentChar character)
{
    switch (character)
    {
        case SEG_DASH:
            return kDashPattern;
        case SEG_ERROR:
            return kErrorPattern;
        case SEG_BLANK:
        default:
            return blank();
    }
}

uint8_t SegmentEncoder::blank()
{
    return kBlankPattern;
}

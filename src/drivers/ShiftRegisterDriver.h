#pragma once

#include <stdint.h>

#include "common/Status.h"

class ShiftRegisterDriver
{
public:
    StatusCode begin();

    void shiftOut(uint8_t segmentData, uint8_t digitData);
    void latch();
    void setOutputEnable(bool enable);
};

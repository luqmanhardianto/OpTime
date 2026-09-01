#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "config/PinConfig.h"

class GpioHal
{
public:
    StatusCode begin();

    void pinModeInput(uint8_t pin);
    void pinModeOutput(uint8_t pin);

    bool read(uint8_t pin);
    void write(uint8_t pin, bool state);
    void writeActiveLow(uint8_t pin, bool enable);
};

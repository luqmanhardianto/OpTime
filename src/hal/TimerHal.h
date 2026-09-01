#pragma once

#include <stdint.h>

#include "common/Status.h"

class TimerHal
{
public:
    StatusCode begin();

    uint32_t millis();
    uint32_t micros();

    void attachTickCallback(void (*callback)());

private:
    static void (*tickCallback_)();
};

#pragma once

#include <stdint.h>

enum class StatusCode : uint8_t
{
    OK = 0,
    ERROR,
    INVALID_PARAMETER,
    NOT_READY,
    TIMEOUT,
    BUSY,
    NO_RESOURCE,
    NO_CHANGE
};

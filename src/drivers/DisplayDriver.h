#pragma once

#include <stdint.h>

#include "common/Status.h"
#include "drivers/SegmentEncoder.h"
#include "drivers/ShiftRegisterDriver.h"

class DisplayDriver
{
public:
    StatusCode begin();
    void enableRefresh();
    void disableRefresh();
    bool isRefreshEnabled() const;

    void setDigit(uint8_t index, uint8_t value);
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    void setColon(bool state);
    void clear();
    void refreshISR();
    void refreshBrightnessISR();
    void refreshDirect();
    void swapBuffer();
    void setBrightness(uint8_t level);

private:
    static constexpr uint8_t kDigitCount = 6U;
    static constexpr uint8_t kBufferSize = 6U;

    uint8_t frontBuffer_[kBufferSize];
    uint8_t backBuffer_[kBufferSize];
    ShiftRegisterDriver shiftRegister_;
    uint8_t currentDigit_;
    uint8_t brightnessLevel_;
    bool colonEnabled_;
    volatile bool initialized_;
    volatile bool refreshEnabled_;

    uint8_t digitMaskForIndex(uint8_t index) const;
    uint8_t segmentValueForDigit(uint8_t value) const;
};

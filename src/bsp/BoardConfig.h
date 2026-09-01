#pragma once

#include <Arduino.h>

#include <stdint.h>

#ifdef DISPLAY
#undef DISPLAY
#endif
#ifdef BUTTON
#undef BUTTON
#endif
#ifdef LED
#undef LED
#endif
#ifdef BUZZER
#undef BUZZER
#endif
#ifdef RTC
#undef RTC
#endif

namespace Board
{
enum class HardwareRevision : uint8_t
{
    REV_A = 0U,
    REV_B = 1U
};

constexpr HardwareRevision HARDWARE_REVISION = HardwareRevision::REV_A;
constexpr uint8_t BOARD_MAJOR = 1U;
constexpr uint8_t BOARD_MINOR = 0U;
constexpr uint8_t BOARD_PATCH = 0U;

namespace Pin
{
constexpr uint8_t RTC_SQW = 2U;
constexpr uint8_t BUZZER = 3U;
constexpr uint8_t BUTTON_POWER = 4U;
constexpr uint8_t BUTTON_SELECT = 5U;
constexpr uint8_t BUTTON_NEXT = 6U;
constexpr uint8_t BUTTON_UP = 7U;
constexpr uint8_t BUTTON_DOWN = 8U;
constexpr uint8_t DISPLAY_OE = 9U;
constexpr uint8_t DISPLAY_LATCH = 10U;
constexpr uint8_t DISPLAY_DATA = 11U;
constexpr uint8_t POWER_LED = 12U;
constexpr uint8_t DISPLAY_CLOCK = 13U;
constexpr uint8_t I2C_SDA = A4;
constexpr uint8_t I2C_SCL = A5;
}  // namespace Pin

namespace Display
{
constexpr uint8_t DIGIT_COUNT = 6U;
constexpr uint8_t REGISTER_COUNT = 2U;
}  // namespace Display

namespace Electrical
{
constexpr uint8_t BUTTON_ACTIVE_LEVEL = LOW;
constexpr uint8_t BUZZER_ACTIVE_LEVEL = LOW;
constexpr uint8_t LED_ACTIVE_LEVEL = LOW;
constexpr uint8_t DISPLAY_OE_ACTIVE_LEVEL = LOW;
constexpr uint8_t DISPLAY_OE_INACTIVE_LEVEL = HIGH;
}  // namespace Electrical

namespace Capability
{
enum : uint8_t
{
    CAP_RTC = 1U << 0,
    CAP_DISPLAY = 1U << 1,
    CAP_BUTTONS = 1U << 2,
    CAP_BUZZER = 1U << 3,
    CAP_POWER_LED = 1U << 4
};
}  // namespace Capability

constexpr uint8_t BOARD_CAPABILITY_MASK = Capability::CAP_RTC | Capability::CAP_DISPLAY |
                                         Capability::CAP_BUTTONS | Capability::CAP_BUZZER |
                                         Capability::CAP_POWER_LED;
}  // namespace Board

#include <unity.h>

#include "bsp/BoardConfig.h"

void test_board_identity_and_revision(void)
{
    TEST_ASSERT_EQUAL_UINT8(1U, Board::BOARD_MAJOR);
    TEST_ASSERT_EQUAL_UINT8(0U, Board::BOARD_MINOR);
    TEST_ASSERT_EQUAL_UINT8(0U, Board::BOARD_PATCH);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(Board::HardwareRevision::REV_A),
                           static_cast<uint8_t>(Board::HARDWARE_REVISION));
}

void test_board_pin_mapping(void)
{
    TEST_ASSERT_EQUAL_UINT8(2U, Board::Pin::RTC_SQW);
    TEST_ASSERT_EQUAL_UINT8(3U, Board::Pin::BUZZER);
    TEST_ASSERT_EQUAL_UINT8(4U, Board::Pin::BUTTON_POWER);
    TEST_ASSERT_EQUAL_UINT8(5U, Board::Pin::BUTTON_SELECT);
    TEST_ASSERT_EQUAL_UINT8(6U, Board::Pin::BUTTON_NEXT);
    TEST_ASSERT_EQUAL_UINT8(7U, Board::Pin::BUTTON_UP);
    TEST_ASSERT_EQUAL_UINT8(8U, Board::Pin::BUTTON_DOWN);
    TEST_ASSERT_EQUAL_UINT8(9U, Board::Pin::DISPLAY_OE);
    TEST_ASSERT_EQUAL_UINT8(10U, Board::Pin::DISPLAY_LATCH);
    TEST_ASSERT_EQUAL_UINT8(11U, Board::Pin::DISPLAY_DATA);
    TEST_ASSERT_EQUAL_UINT8(12U, Board::Pin::POWER_LED);
    TEST_ASSERT_EQUAL_UINT8(13U, Board::Pin::DISPLAY_CLOCK);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(A4), Board::Pin::I2C_SDA);
    TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(A5), Board::Pin::I2C_SCL);
}

void test_board_capability_mask(void)
{
    TEST_ASSERT_EQUAL_UINT8(0x1FU, Board::BOARD_CAPABILITY_MASK);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_board_identity_and_revision);
    RUN_TEST(test_board_pin_mapping);
    RUN_TEST(test_board_capability_mask);
    return UNITY_END();
}

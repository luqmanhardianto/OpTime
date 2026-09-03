#pragma once

// Temporary Timer1/display diagnostic. Set to 0 for normal firmware operation.
#define OPTIME_TIMER1_DIAGNOSTIC 0

// Set to 1 to disable Timer1 during the measurement, or 0 to test with it on.
#define OPTIME_TIMER1_DIAGNOSTIC_TIMER1_OFF 1

// Diagnostic reports are sent after the measurement; normal firmware remains silent.
#define OPTIME_TIMER1_DIAGNOSTIC_SERIAL 1

// Use 600000UL for 10 minutes or 3600000UL for 60 minutes.
#define OPTIME_TIMER1_DIAGNOSTIC_DURATION_MS 1800000UL

// Independent RTC-vs-millis diagnostic. Set to 1 only for measurement.
#define OPTIME_MILLIS_RTC_DIAGNOSTIC 0

// RTC resolution is one second; use 600UL for 10 minutes or 3600UL for 60 minutes.
#define OPTIME_MILLIS_RTC_DIAGNOSTIC_DURATION_SECONDS 1800UL

// Set to 1 to disable Timer1 during the RTC-vs-millis measurement.
#define OPTIME_MILLIS_RTC_DIAGNOSTIC_TIMER1_OFF 1
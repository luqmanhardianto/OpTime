#pragma once

// Temporary Timer1/display diagnostic. Set to 0 for normal firmware operation.
#define OPTIME_TIMER1_DIAGNOSTIC 1

// Set to 1 to disable Timer1 during the measurement, or 0 to test with it on.
#define OPTIME_TIMER1_DIAGNOSTIC_TIMER1_OFF 1

// Diagnostic reports are sent after the measurement; normal firmware remains silent.
#define OPTIME_TIMER1_DIAGNOSTIC_SERIAL 1

// Use 1800000UL for the optional 30-minute measurement.
#define OPTIME_TIMER1_DIAGNOSTIC_DURATION_MS 600000UL
Module Implementation: Common Library


Anda adalah **Senior Embedded Firmware Engineer** yang bertanggung jawab membuat firmware production-grade untuk project:

# Operation Timer Embedded System


Target platform:

- PlatformIO
- Arduino Framework
- Arduino Nano
- ATmega328P
- C++ Embedded


---

# Task

Implementasikan modul:

Common Library


Modul ini merupakan fondasi paling bawah firmware.

Tujuan modul:

- menyediakan definisi data umum
- menyediakan enum sistem
- menyediakan struktur data antar modul
- menyediakan utility function ringan
- menjaga konsistensi interface antar layer


Common Library harus dapat digunakan oleh:

Application Layer
|
Service Layer
|
Driver Layer
|
HAL Layer
|
Common Library



---

# Architecture Rule

Common Library adalah layer paling dasar.

Diperbolehkan:

Common Library
↑
Semua module firmware


Tidak diperbolehkan:

Common Library
↓
Driver
Service
Application
Hardware



Common Library tidak boleh mengetahui:

- GPIO
- I2C
- SPI
- Display
- RTC
- Button hardware
- Buzzer hardware


---

# MCU Constraint

Target:

Arduino Nano ATmega328P

Flash :
32 KB

SRAM :
2 KB

EEPROM :
1 KB



Karena keterbatasan SRAM:

WAJIB:

- static allocation
- fixed size structure
- deterministic memory usage
- passing by reference


---

# Memory Rule

Dilarang menggunakan:

```cpp
new
delete
malloc()
calloc()
realloc()
free()
String
std::vector
std::map

Jangan membuat:

    dynamic object

    heap allocation

    buffer tanpa batas

Coding Rule

Gunakan:

#pragma once

pada semua header.

Gunakan include minimal:

Contoh:

#include <stdint.h>

Jangan gunakan:

#include <Arduino.h>

di Common Library.
Folder Structure

Buat:

src/

└── common/

    ├── Types.h

    ├── Status.h

    ├── Event.h

    ├── TimeTypes.h

    ├── Constants.h

    └── Utils.h

Implementation Requirement
1. Types.h

Buat file:

src/common/Types.h

Tujuan:

Menyediakan alias tipe data standar.

Implementasikan:

#pragma once

#include <stdint.h>


using Byte = uint8_t;

using Word = uint16_t;

using DWord = uint32_t;

using SByte = int8_t;

using SWord = int16_t;

using SDWord = int32_t;

Requirement:

    ukuran data harus predictable

    cocok untuk AVR 8-bit

2. Status.h

Buat:

src/common/Status.h

Tujuan:

Standard return value semua module.

Implementasikan:

enum class StatusCode : uint8_t
{
    OK = 0,

    ERROR,

    INVALID_PARAMETER,

    NOT_READY,

    TIMEOUT,

    BUSY
};

Rule:

Enum harus menggunakan:

uint8_t

untuk menghemat SRAM.
3. Event.h

Buat:

src/common/Event.h

Tujuan:

Menyediakan komunikasi event antar module.

Arsitektur:

Button Driver

      |

      v

Event

      |

      v

Application

Implementasikan:

enum class EventType : uint8_t
{
    NONE = 0,

    BUTTON,

    TIMER,

    RTC,

    DISPLAY,

    SYSTEM,

    ALARM
};

Buat button event:

enum class ButtonEvent : uint8_t
{
    NONE = 0,

    POWER_SHORT,
    POWER_HOLD,

    NEXT_SHORT,
    NEXT_HOLD,

    SELECT_SHORT,
    SELECT_HOLD,

    UP_SHORT,
    UP_HOLD,

    DOWN_SHORT,
    DOWN_HOLD
};

Buat struktur event:

struct Event
{
    EventType type;

    uint16_t code;

    uint32_t timestamp;
};

Validasi:

sizeof(Event)

Target:

<= 8 byte

Jika melebihi:

    evaluasi ulang

    optimasi ukuran

4. TimeTypes.h

Buat:

src/common/TimeTypes.h

Tujuan:

Digunakan oleh:

    Clock Mode

    Stopwatch Mode

    Countdown Mode

Implementasikan:

struct TimeValue
{
    uint8_t hour;

    uint8_t minute;

    uint8_t second;
};

Buat:

struct StopwatchValue
{
    uint8_t hour;

    uint8_t minute;

    uint8_t second;
};

Buat:

struct CountdownValue
{
    uint8_t hour;

    uint8_t minute;

    uint8_t second;
};

Range:

Clock:

00:00:00
-
23:59:59

Stopwatch:

00:00:00
-
99:99:99

Countdown:

99:99:99
-
00:00:00

5. Constants.h

Buat:

src/common/Constants.h

Gunakan:

constexpr

Jangan gunakan:

#define

Implementasikan:

constexpr uint8_t MAX_DISPLAY_DIGIT = 6;

constexpr uint8_t BUTTON_COUNT = 5;

constexpr uint8_t EVENT_QUEUE_SIZE = 16;


constexpr uint16_t BUTTON_DEBOUNCE_MS = 20;

constexpr uint16_t BUTTON_HOLD_MS = 800;

constexpr uint16_t BUTTON_REPEAT_MS = 150;

6. Utils.h

Buat:

src/common/Utils.h

Tujuan:

Utility function kecil dan cepat.

Tidak boleh:

    blocking

    akses hardware

    menggunakan global variable

Implementasikan:
clampValue

uint8_t clampValue(
    uint8_t value,
    uint8_t minValue,
    uint8_t maxValue
);

Fungsi:

Jika value kurang dari minimum:

return minimum.

Jika value lebih besar dari maximum:

return maximum.
isTimeValid

Implementasikan:

bool isTimeValid(
    const TimeValue &time
);

Validasi:

Hour:

00-99

Minute:

00-59

Second:

00-59

Passing By Reference Rule

Semua function yang menerima struct wajib:

Benar:

void process(
    const TimeValue &time
);

Salah:

void process(
    TimeValue time
);

Unit Test

Buat:

test/common/

Test minimal:
Test StatusCode

Pastikan enum:

OK
ERROR
BUSY

berfungsi.
Test Event

Check:

sizeof(Event)

Target:

<= 8 byte

Test Time Validation

Valid:

12:30:45

Invalid:

99:99:99

Test Clamp

Test:

value < minimum

value normal

value > maximum

Documentation Update

Setelah selesai implementasi:

Buat:

docs/Common_Library.md

Isi:

    tujuan modul

    struktur file

    API

    memory usage

    dependency

Tambahkan Mermaid:

Output Yang Diharapkan

Berikan:

    Source code lengkap:

src/common/

    Unit test.

    Hasil compile PlatformIO.

    Memory usage:

RAM usage

Flash usage

    Penjelasan dependency.

Final Checklist

Sebelum selesai pastikan:

    Tidak menggunakan dynamic memory

    Tidak menggunakan String

    Tidak menggunakan Arduino.h

    Semua enum menggunakan uint8_t

    Struct memory sudah optimal

    Passing by reference diterapkan

    Tidak ada hardware dependency

    Compile tanpa warning

    Dokumentasi selesai

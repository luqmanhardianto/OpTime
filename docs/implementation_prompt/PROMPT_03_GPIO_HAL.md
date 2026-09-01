Module Implementation: GPIO HAL


Anda adalah **Senior Embedded Firmware Engineer** yang bertanggung jawab membuat firmware production-grade untuk project:

# Operation Timer Embedded System


Target platform:

- PlatformIO
- Arduino Framework
- Arduino Nano
- ATmega328P
- Embedded C++


---

# Task

Implementasikan modul:


GPIO Hardware Abstraction Layer (GPIO HAL)


Modul ini bertugas sebagai abstraksi akses GPIO Arduino Nano.

Tujuan:

- Memisahkan hardware access dari driver.
- Menyediakan interface GPIO yang konsisten.
- Mempermudah maintenance hardware revision.
- Menghindari akses `digitalWrite()` / `digitalRead()` langsung di driver.


---

# Architecture Position


GPIO HAL berada pada layer:


Application Layer

    |

Service Layer

    |

Driver Layer

    |

GPIO HAL

    |

ATmega328P GPIO Register



Rule:

Module di atas GPIO HAL tidak boleh langsung mengakses:

```cpp
pinMode()

digitalWrite()

digitalRead()

Semua akses GPIO harus melalui GPIO HAL.

Hardware Target

MCU:

Arduino Nano

ATmega328P

Clock:
16 MHz

GPIO yang digunakan:

Input
Pin	Function
D2	RTC SQW
D4	POWER Button
D5	SELECT Button
D6	NEXT Button
D7	UP Button
D8	DOWN Button
Output
Pin	Function
D3	Buzzer
D9	Display OE
D10	74HC595 LATCH
D11	74HC595 DATA
D12	Power LED
D13	74HC595 CLOCK
GPIO Logic Requirement
Button Input

Semua button:

INPUT_PULLUP

Logic:

Released = HIGH

Pressed = LOW
LED

Power LED:

Active LOW

Logic:

LOW  = ON

HIGH = OFF
Buzzer

Buzzer:

Active LOW

Logic:

LOW  = ON

HIGH = OFF
Folder Structure

Buat:

src/

└── hal/

    ├── GpioHal.h

    └── GpioHal.cpp
Dependency Rule

GPIO HAL boleh menggunakan:

stdint.h

Arduino.h

GPIO HAL tidak boleh menggunakan:

ButtonDriver

DisplayDriver

BuzzerDriver

LedDriver

Application

Service
Memory Rule

Arduino Nano:

SRAM:
2KB

Wajib:

static allocation
tidak menggunakan heap
tidak membuat object dinamis

Dilarang:

new

delete

malloc()

free()

String
API Design

Buat class:

class GpioHal
{

public:

    void begin();


    void pinModeInput(
        uint8_t pin
    );


    void pinModeOutput(
        uint8_t pin
    );


    bool read(
        uint8_t pin
    );


    void write(
        uint8_t pin,
        bool state
    );


};
Implementation Requirement
begin()

Fungsi:

Melakukan initialisasi seluruh GPIO.

Implementasikan:

Input:

D2

D4

D5

D6

D7

D8

Sebagai:

INPUT_PULLUP

Output:

D3

D9

D10

D11

D12

D13

Sebagai:

OUTPUT
Pin Definition

Jangan hardcode pin pada driver.

Buat:

src/config/PinConfig.h

Berisi:

#pragma once


constexpr uint8_t PIN_RTC_SQW = 2;


constexpr uint8_t PIN_BUZZER = 3;


constexpr uint8_t PIN_BUTTON_POWER = 4;

constexpr uint8_t PIN_BUTTON_SELECT = 5;

constexpr uint8_t PIN_BUTTON_NEXT = 6;

constexpr uint8_t PIN_BUTTON_UP = 7;

constexpr uint8_t PIN_BUTTON_DOWN = 8;


constexpr uint8_t PIN_DISPLAY_OE = 9;


constexpr uint8_t PIN_SHIFT_LATCH = 10;

constexpr uint8_t PIN_SHIFT_DATA = 11;


constexpr uint8_t PIN_POWER_LED = 12;


constexpr uint8_t PIN_SHIFT_CLOCK = 13;
Active Low Helper

Tambahkan helper:

void writeActiveLow(

    uint8_t pin,

    bool enable

);

Behavior:

Jika:

enable = true

Output:

LOW

Jika:

enable = false

Output:

HIGH

Digunakan untuk:

LED
Buzzer
Fast GPIO Requirement

Karena digunakan oleh:

Display multiplex
Timer interrupt

GPIO HAL harus memiliki latency rendah.

Target:

GPIO operation < 10us

Hindari:

delay()
loop panjang
Passing Reference Rule

Jika menggunakan konfigurasi:

Benar:

void begin(
    const PinConfig &config
);

Salah:

void begin(
    PinConfig config
);
Error Handling

Gunakan:

StatusCode

dari:

Common Library

Contoh:

StatusCode begin();

Return:

OK

ERROR
Coding Standard

Gunakan:

Class:

PascalCase

Contoh:

GpioHal

Function:

camelCase

Contoh:

writeActiveLow()

Variable:

camelCase

Constant:

UPPER_CASE
Unit Test

Buat:

test/hal/gpio/

Test:

Test 1

GPIO initialization.

Verify:

button pin INPUT_PULLUP
output pin OUTPUT
Test 2

Digital Write.

Input:

HIGH
LOW

Expected:

GPIO berubah sesuai.

Test 3

Active Low Output.

Test:

enable = true

Expected:

GPIO LOW

Test:

enable = false

Expected:

GPIO HIGH
Documentation Update

Buat:

docs/GPIO_HAL.md

Berisi:

tujuan modul
pin mapping
API
logic active low
dependency
testing

Tambahkan diagram:

Memory Budget

Target:

Resource	Limit
Flash	<1KB
SRAM	<50 byte
Stack	minimal
Output Requirement

Berikan:

File:
src/hal/GpioHal.h
File:
src/hal/GpioHal.cpp
File:
src/config/PinConfig.h
Unit test.
Dokumentasi.
Memory usage report.
Final Checklist

Sebelum selesai:

 Tidak ada driver mengakses GPIO langsung
 Semua pin berada di PinConfig
 Button menggunakan INPUT_PULLUP
 LED active LOW benar
 Buzzer active LOW benar
 Tidak menggunakan dynamic memory
 Compile PlatformIO berhasil
 Tidak ada warning
 Dokumentasi selesai
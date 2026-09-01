Module Implementation: Timer HAL


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


Timer Hardware Abstraction Layer (Timer HAL)


Modul ini bertanggung jawab menyediakan layanan timer dasar untuk seluruh firmware.


Timer HAL digunakan oleh:

- Display Multiplex Driver
- Scheduler
- Button Driver
- Time Service
- Stopwatch
- Countdown
- Diagnostic System


---

# Objective

Menyediakan sumber waktu yang:

- stabil
- deterministic
- low overhead
- interrupt based
- tidak blocking


Timer HAL harus menyediakan:

1. Millisecond system tick
2. Periodic callback trigger
3. Microsecond timing reference (jika diperlukan)
4. Timer interrupt abstraction


---

# Architecture Position


Timer HAL berada pada layer:



Application Layer

    |

Service Layer

    |

Driver Layer

    |

Timer HAL

    |

ATmega328P Timer Peripheral



Rule:

Module di atas Timer HAL tidak boleh mengakses:

```cpp
millis()

micros()

delay()

Timer Register langsung

Semua kebutuhan timing harus melalui Timer HAL.

Hardware Target

MCU:

ATmega328P

Clock:
16 MHz

Timer yang tersedia:

Timer0
Timer1
Timer2
Timer Allocation Rule

Gunakan:

Timer1

Prioritas utama:

Display Multiplex Refresh

Alasan:

16 bit timer
resolusi tinggi
stabil untuk interrupt
Timer2

Untuk:

System Tick

atau:

Scheduler Tick
Timer0

Jangan digunakan.

Alasan:

Arduino framework menggunakan Timer0 untuk:

millis()
delay()
micros()
Recommended Architecture

Gunakan:

Timer2 Interrupt

        |

        v

Timer HAL Tick

        |

        +----------------+

        |                |

        v                v

Scheduler          Button Timing


Timer1:

Timer1 Interrupt

        |

        v

Display Multiplex ISR

Folder Structure

Buat:

src/

└── hal/

    ├── GpioHal.h

    ├── GpioHal.cpp

    ├── TimerHal.h

    └── TimerHal.cpp
Dependency Rule

Timer HAL boleh menggunakan:

stdint.h

avr/io.h

avr/interrupt.h

common/Status.h

Tidak boleh menggunakan:

DisplayDriver

Scheduler

ButtonDriver

ModeManager

Application
Memory Rule

Arduino Nano:

SRAM:
2048 byte

WAJIB:

static allocation
tidak menggunakan heap
callback pointer sederhana
tidak menggunakan STL

Dilarang:

new

delete

malloc()

free()

std::function

lambda capture
Timer API Design

Implementasikan:

class TimerHal
{

public:

    StatusCode begin();


    uint32_t millis();


    uint32_t micros();


    void attachTickCallback(
        void (*callback)()
    );


};
ISR Safety Rule

Interrupt Service Routine:

WAJIB:

cepat
tidak blocking
tidak melakukan operasi berat

Dilarang:

Serial.print()

delay()

dynamic allocation

loop panjang
Tick Callback Rule

Callback digunakan untuk:

Scheduler
Event generator

Contoh:

void schedulerTick()
{
    scheduler.tick();
}

Timer HAL hanya memanggil callback.

Tidak boleh mengetahui:

Scheduler class
Timer Tick Resolution

Minimum requirement:

1ms tick

Target:

accuracy ±1ms
Timer Configuration

Implementasikan:

Timer2

Mode:

CTC Mode

Target:

Interrupt setiap 1ms

Perhitungan harus menggunakan:

F_CPU = 16MHz

Gunakan:

prescaler
OCR value

dengan nilai yang tepat.

Critical Section Rule

Jika membaca variable yang diubah ISR:

WAJIB:

gunakan atomic protection.

Contoh:

ATOMIC_BLOCK(
    ATOMIC_RESTORESTATE
)
{

}
Shared Variable Rule

Variable yang digunakan ISR:

Gunakan:

volatile

Contoh:

volatile uint32_t systemTick;
API Behavior
begin()

Melakukan:

konfigurasi timer
enable interrupt
reset counter

Return:

StatusCode::OK
millis()

Mengembalikan:

system uptime milliseconds

Example:

uint32_t now = timer.millis();
micros()

Mengembalikan:

microsecond counter

Digunakan untuk:

diagnostic
profiling
Callback Handling

Simpan:

static function pointer

Contoh:

static void (*tickCallback)();

Jangan gunakan:

std::function
Coding Rule

Class:

PascalCase

Example:

TimerHal

Function:

camelCase

Example:

attachTickCallback()

Variable:

camelCase

Constant:

UPPER_CASE
Passing Reference Rule

Jika menggunakan configuration:

Benar:

StatusCode begin(
    const TimerConfig &config
);

Salah:

StatusCode begin(
    TimerConfig config
);
Integration Requirement

Timer HAL harus siap digunakan oleh:

Scheduler
Timer HAL

   |

   v

Scheduler Tick

   |

   v

Task Execution
Display Driver
Timer1

 |

 v

Display Multiplex ISR

 |

 v

6 Digit Refresh
Unit Test

Buat:

test/hal/timer/
Test 1

Initialization

Verify:

timer configured
interrupt enabled
Test 2

Tick Counter

Run:

1000 tick

Expected:

1000 ms
Test 3

Callback

Register:

callback()

Verify:

callback dipanggil setiap tick.

Test 4

Atomic Access

Verify:

tidak terjadi corrupt data
ISR safe
Documentation Update

Buat:

docs/Timer_HAL.md

Berisi:

timer allocation
interrupt flow
API
timing accuracy
memory usage

Tambahkan Mermaid:

Memory Budget

Target:

Resource	Limit
Flash	<2KB
SRAM	<50 byte
ISR execution	<50us
Output Requirement

Berikan:

File:
src/hal/TimerHal.h
File:
src/hal/TimerHal.cpp
Timer register configuration.
Unit test.
Timing calculation.
Memory usage report.
Documentation.
Final Checklist

Sebelum selesai:

 Timer0 tidak digunakan
 Timer1 reserved untuk display
 Timer2 digunakan system tick
 ISR tidak blocking
 Tidak memakai dynamic memory
 Tidak memakai delay()
 Variable ISR menggunakan volatile
 Atomic protection diterapkan
 Compile PlatformIO sukses
 Dokumentasi selesai
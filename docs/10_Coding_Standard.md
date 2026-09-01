# 10 - Coding Standard

> Coding Standard Specification for Operation Timer Firmware

**Document ID** : OT-DOC-010  
**Document Name** : Coding Standard  
**Project** : Operation Timer  
**Version** : 2.0.0  
**Status** : Production Standard  
**Last Update** : 2026-07-30

> **Architecture Note**
>
> Dokumen ini merupakan standar wajib seluruh source code firmware. Seluruh developer harus mengikuti aturan pada dokumen ini agar kualitas firmware tetap konsisten, mudah diuji, dan siap diproduksi.

---

# Revision History

| Version | Date | Author | Description |
|----------|------------|----------------|----------------------------|
| 1.0.0 | 2026-07-30 | Development Team | Initial Coding Standard |
| 2.0.0 | 2026-07-30 | Development Team | Production Ready Standard |

---

# 1. Purpose

Dokumen ini mendefinisikan standar penulisan source code firmware Operation Timer.

Tujuan utama:

- Konsistensi kode
- Mudah dipelihara
- Mudah diuji
- Mudah direview
- Mengurangi bug
- Menghemat SRAM dan Flash
- Siap diproduksi massal

---

# 2. Coding Philosophy

Firmware mengikuti prinsip berikut:

- SOLID Principle
- Single Responsibility Principle (SRP)
- KISS (Keep It Simple)
- DRY (Don't Repeat Yourself)
- Zero Dynamic Memory
- Deterministic Execution
- Event Driven
- Hardware Abstraction
- Low Memory Optimization

---

# 3. Compiler Configuration

PlatformIO wajib menggunakan:

```ini
build_flags =
    -Wall
    -Wextra
    -Werror
    -std=gnu++17
```

Seluruh warning dianggap sebagai error.

Build harus bersih (Zero Warning).

---

# 4. File Organization

```
Firmware

src/
    app/
    services/
    drivers/
    hal/

include/

config/

docs/

test/
```

Tidak diperbolehkan meletakkan source secara acak.

---

# 5. File Size Rule

| File | Maximum |
|-------|----------|
| Header (.h) | 200 lines |
| Source (.cpp) | 300 lines |

Jika melebihi batas, file harus dipecah.

---

# 6. Function Size Rule

Satu fungsi maksimal

```
40 baris
```

Ideal

```
15~25 baris
```

Fungsi panjang harus dipecah menjadi fungsi yang lebih kecil.

---

# 7. Class Responsibility

Satu class hanya memiliki **satu tanggung jawab**.

Contoh:

```
DisplayDriver

✔ Display Only
```

```
RtcDriver

✔ RTC Only
```

```
DisplayDriver

✘ RTC
✘ Button
✘ Buzzer
```

---

# 8. Naming Convention

## Class

Gunakan PascalCase.

```cpp
DisplayDriver

ButtonManager

RtcDriver
```

---

## Struct

Gunakan PascalCase.

```cpp
DisplayBuffer

ButtonEvent

Time
```

---

## Enum

Gunakan enum class.

```cpp
enum class Mode
{
    Clock,
    Stopwatch,
    Countdown
};
```

---

## Function

Gunakan camelCase.

```cpp
updateDisplay()

readTime()

handleEvent()
```

---

## Variable

Gunakan camelCase.

```cpp
currentMode

displayBuffer

buttonState
```

---

## Member Variable

Gunakan prefix

```
m_
```

```cpp
m_display

m_mode

m_state
```

---

## Constant

Gunakan prefix

```
k
```

```cpp
kRefreshRate

kDebounceTime

kBrightness
```

---

## Macro

Macro hanya digunakan bila benar-benar diperlukan.

Format:

```cpp
PROJECT_NAME
```

Contoh:

```cpp
F_CPU
```

Macro untuk konstanta biasa **dilarang**.

---

# 9. constexpr Rule

Prioritas penggunaan:

```
constexpr

↓

const

↓

macro
```

Contoh:

```cpp
constexpr uint8_t kDigits = 6;
```

Bukan

```cpp
#define DIGITS 6
```

---

# 10. enum class Rule

Selalu gunakan

```cpp
enum class
```

Tidak diperbolehkan menggunakan enum biasa.

---

# 11. Passing by Reference Rule

Semua object lebih besar dari 4 Byte wajib dikirim menggunakan referensi.

Benar:

```cpp
void setTime(const Time& time);

void show(const DisplayBuffer& buffer);

void push(const ButtonEvent& event);
```

Salah:

```cpp
void setTime(Time time);

void push(ButtonEvent event);
```

---

# 12. Const Correctness

Semua fungsi yang tidak mengubah object wajib diberi

```cpp
const
```

Contoh:

```cpp
Mode mode() const;

bool isRunning() const;
```

---

# 13. Override Rule

Virtual function wajib menggunakan

```cpp
override
```

Contoh:

```cpp
void update() override;
```

---

# 14. noexcept Rule

Fungsi yang dijamin tidak menghasilkan exception sebaiknya menggunakan

```cpp
noexcept
```

Contoh:

```cpp
void update() noexcept;
```

---

# 15. Return Value Rule

Return value tidak digunakan untuk menyampaikan banyak status.

Disarankan:

```cpp
enum class Status
{
    Ok,
    Error,
    Timeout,
    InvalidData
};
```

Bukan:

```cpp
bool write();
```

Melainkan:

```cpp
Status write();
```

---

# 16. Boolean Rule

Boolean hanya digunakan untuk kondisi logika.

Contoh:

```cpp
isRunning

isConnected

isPressed
```

Tidak digunakan untuk kode error.

---

# 17. Memory Allocation Rule

Dilarang menggunakan

```cpp
malloc()

calloc()

realloc()

free()

new

delete
```

Seluruh memory harus:

```
Static Allocation
```

---

# 18. String Rule

Tidak diperbolehkan menggunakan

```cpp
String
```

Gunakan:

```cpp
char[]

constexpr char
```

---

# 19. Magic Number Rule

Magic Number dilarang.

Benar:

```cpp
constexpr uint16_t kDebounceTime = 30;
```

Salah:

```cpp
if(time > 30)
```

---

# 20. Header Rule

Gunakan

```cpp
#pragma once
```

Tidak menggunakan include guard manual.

---

# 21. Include Rule

Urutan include:

```cpp
#include "OwnHeader.h"

#include <Arduino.h>

#include <Wire.h>

#include "OtherModule.h"
```

---

# 22. Dependency Rule

Dependency diperbolehkan:

```
Application

↓

Service

↓

Driver

↓

HAL
```

Dependency ke atas dilarang.

---

# 23. ISR Rule

ISR hanya boleh:

- Set Flag
- Refresh Display
- Exit

ISR dilarang:

- Serial
- Wire
- EEPROM
- delay()
- malloc()
- new

---

# 24. Display Rule

ISR hanya membaca

```
Front Buffer
```

Application hanya menulis

```
Back Buffer
```

Swap dilakukan secara atomik.

---

# 25. Event Rule

Komunikasi antar modul menggunakan Event.

Tidak boleh:

```cpp
ModeManager

↓

ButtonDriver
```

Harus:

```text
ButtonDriver

↓

EventQueue

↓

ModeManager
```

---

# 26. Scheduler Rule

Semua task menggunakan Scheduler.

Tidak diperbolehkan:

```cpp
delay()
```

---

# 27. Logging Rule

Debug hanya menggunakan:

```cpp
Serial
```

Debug harus dapat dimatikan melalui:

```cpp
FirmwareConfig.h
```

Contoh:

```cpp
constexpr bool kEnableDebug = false;
```

---

# 28. Comment Rule

Seluruh API publik wajib menggunakan komentar Doxygen.

Contoh:

```cpp
/**
 * @brief Update display buffer
 *
 * @param buffer Display buffer
 */
void update(const DisplayBuffer& buffer);
```

Komentar harus menjelaskan **mengapa**, bukan hanya **apa**.

---

# 29. Formatting Rule

Indentasi:

```
4 Spaces
```

Tidak menggunakan TAB.

Brace Style:

```cpp
if (...)
{
}
```

---

# 30. clang-format

Seluruh project menggunakan file

```
.clang-format
```

Format harus dijalankan sebelum commit.

---

# 31. clang-tidy

Project menggunakan

```
.clang-tidy
```

Minimal pemeriksaan:

- readability
- performance
- modernize
- bugprone

---

# 32. Git Rule

Satu commit hanya untuk satu perubahan logis.

Contoh:

```
Fix debounce timing

Add Countdown Mode

Refactor Display Driver
```

---

# 33. Branch Rule

```
main

develop

feature/*

bugfix/*

release/*
```

---

# 34. Version Rule

Versi firmware hanya berasal dari

```
Version.h
```

Tidak boleh ada versi hardcoded.

---

# 35. Unit Test Rule

Seluruh Driver wajib dapat diuji tanpa hardware.

Gunakan mock:

- GPIO
- RTC
- Display
- Scheduler

---

# 36. PlatformIO Rule

PlatformIO menjadi satu-satunya sistem build resmi.

Gunakan:

```
platformio.ini
```

Konfigurasi tidak boleh tersebar di source code.

---

# 37. Documentation Rule

Seluruh dokumentasi berada pada:

```
docs/
```

Menggunakan:

- Markdown
- Mermaid

Tidak menggunakan dokumen proprietary.

---

# 38. Production Checklist

Sebelum Release:

- ☐ Zero Warning
- ☐ Unit Test Pass
- ☐ Version Update
- ☐ Changelog Update
- ☐ Documentation Updated
- ☐ Static Analysis Pass
- ☐ Memory Usage Checked
- ☐ Display Refresh Verified
- ☐ RTC Accuracy Verified
- ☐ Countdown Verified

---

# 39. Related Documents

- README.md
- 09_Firmware_Architecture.md
- 11_Project_Structure.md
- 12_Testing_Checklist.md

---

# Implementation Notes

## Recommended Namespace

Seluruh kode berada pada namespace proyek.

```cpp
namespace ot
{
}
```

Menghindari konflik nama library.

---

## Configuration Policy

Semua parameter sistem berada pada folder:

```
config/
```

Contoh:

```
DisplayConfig.h

ButtonConfig.h

FirmwareConfig.h

Version.h
```

---

## Dependency Injection

Class menerima dependency melalui constructor.

```cpp
DisplayDriver(
    ShiftRegister& shiftRegister,
    SegmentEncoder& encoder);
```

Tidak menggunakan object global.

---

## Copy Prevention

Class yang mengelola hardware tidak boleh di-copy.

```cpp
DisplayDriver(const DisplayDriver&) = delete;
DisplayDriver& operator=(const DisplayDriver&) = delete;
```

---

## Static Memory Policy

Target penggunaan SRAM:

```
< 1.2 KB
```

Prioritas:

- constexpr
- PROGMEM
- static
- const reference

---

## PlatformIO Configuration

Disarankan menggunakan:

```ini
lib_ldf_mode = chain+

check_tool = clangtidy

check_flags =
    clangtidy: --checks=*
```

---

## Review Checklist

Setiap Pull Request wajib menjawab:

- Apakah ada dynamic memory?
- Apakah ada magic number?
- Apakah ada delay()?
- Apakah ISR tetap singkat?
- Apakah seluruh object besar menggunakan `const &`?
- Apakah ada warning compiler?
- Apakah dokumentasi ikut diperbarui?

---

# Production Notes

- Seluruh source code harus dapat dikompilasi menggunakan PlatformIO tanpa modifikasi tambahan.
- Tidak diperbolehkan melakukan merge ke branch `main` tanpa code review.
- Setiap perubahan API publik harus disertai pembaruan dokumentasi Markdown dan diagram Mermaid yang relevan.
- Firmware produksi harus lulus unit test, static analysis, dan verifikasi penggunaan Flash/SRAM sebelum proses release.
- Seluruh perubahan yang memengaruhi perilaku sistem wajib dicatat pada `CHANGELOG.md`.

---

**End of Document**
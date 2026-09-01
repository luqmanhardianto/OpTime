# 11 - Project Structure

> Project Directory Structure Specification for Operation Timer Firmware

**Document ID** : OT-DOC-011  
**Document Name** : Project Structure  
**Project** : Operation Timer  
**Version** : 2.0.0  
**Status** : Production Standard  
**Last Update** : 2026-07-30

> **Architecture Note**
>
> Dokumen ini mendefinisikan struktur project PlatformIO yang menjadi standar pengembangan firmware Operation Timer. Seluruh source code, dokumentasi, testing, dan tools harus mengikuti struktur ini.

---

# Revision History

| Version | Date | Author | Description |
|----------|------------|----------------|------------------------------|
|1.0.0|2026-07-30|Development Team|Initial Document|
|2.0.0|2026-07-30|Development Team|Production Ready Project Structure|

---

# 1. Purpose

Dokumen ini mendefinisikan struktur folder firmware agar:

- Konsisten
- Mudah dipelihara
- Mudah dipahami developer baru
- Mudah melakukan testing
- Mudah melakukan release
- Mudah melakukan manufacturing

---

# 2. Development Environment

Firmware menggunakan:

| Item | Standard |
|------|----------|
| IDE | Visual Studio Code |
| Build System | PlatformIO |
| Language | C++17 |
| Compiler | AVR-GCC |
| Framework | Arduino |
| Target MCU | ATmega328P |
| Board | Arduino Nano |

---

# 3. Root Directory

```
OperationTimer/
│
├── .github/
├── .vscode/
├── docs/
├── include/
├── lib/
├── scripts/
├── src/
├── test/
├── tools/
│
├── platformio.ini
├── README.md
├── CHANGELOG.md
├── LICENSE
├── .clang-format
├── .clang-tidy
├── .gitignore
└── Doxyfile
```

---

# 4. Directory Overview

| Folder | Description |
|---------|-------------|
| docs | Project Documentation |
| include | Global Header |
| lib | Reusable Modules |
| src | Main Application |
| test | Unit Test |
| tools | Manufacturing Utilities |
| scripts | Build Automation |
| .github | GitHub Actions |
| .vscode | VSCode Configuration |

---

# 5. Source Tree

```
src/

main.cpp

app/

services/

drivers/

hal/

config/
```

---

# 6. Complete Source Structure

```
src/
│
├── main.cpp
│
├── app/
│   ├── ModeManager.cpp
│   ├── ClockMode.cpp
│   ├── StopwatchMode.cpp
│   ├── CountdownMode.cpp
│   └── FactoryMode.cpp (Future)
│
├── services/
│   ├── Scheduler.cpp
│   ├── TimeService.cpp
│   ├── NotificationManager.cpp
│   ├── EventDispatcher.cpp
│   └── VersionService.cpp
│
├── drivers/
│   ├── DisplayDriver.cpp
│   ├── ShiftRegister.cpp
│   ├── SegmentEncoder.cpp
│   ├── ButtonDriver.cpp
│   ├── RtcDriver.cpp
│   ├── LedDriver.cpp
│   ├── BuzzerDriver.cpp
│   └── EepromDriver.cpp (Future)
│
├── hal/
│   ├── GpioHal.cpp
│   ├── I2cHal.cpp
│   ├── TimerHal.cpp
│   └── EepromHal.cpp
│
└── config/
    ├── BoardConfig.h
    ├── DisplayConfig.h
    ├── ButtonConfig.h
    ├── FirmwareConfig.h
    ├── Version.h
    └── BuildInfo.h (Generated)
```

---

# 7. Library Structure

Seluruh library reusable berada pada folder `lib/`.

```
lib/

Display/

Button/

RTC/

Notification/

Scheduler/

Common/
```

Masing-masing library memiliki struktur:

```
Library/

include/

src/

library.json

README.md
```

Setiap library harus dapat digunakan ulang pada project lain.

---

# 8. Include Folder

Folder `include/` hanya berisi header global.

Contoh:

```
include/

Application.h

Project.h

Types.h

Status.h

Version.h
```

Tidak boleh berisi implementasi.

---

# 9. Config Folder

Seluruh konfigurasi proyek berada pada:

```
src/config/
```

Contoh:

```
BoardConfig.h

DisplayConfig.h

ButtonConfig.h

FirmwareConfig.h

Version.h

PinMap.h

TimingConfig.h
```

Tidak boleh ada konfigurasi tersebar di driver.

---

# 10. Documentation Folder

```
docs/

README.md

00_Project_Overview.md

01_System_Requirements.md

...

20_Release_Process.md

images/

diagrams/
```

Seluruh diagram menggunakan Mermaid.

---

# 11. Test Folder

```
test/

test_button/

test_display/

test_rtc/

test_scheduler/

test_notification/

test_mode/

test_common/
```

Setiap modul wajib memiliki unit test.

---

# 12. Tools Folder

Digunakan untuk utilitas produksi.

```
tools/

font_generator/

segment_table/

version_generator/

release_builder/

memory_report/
```

---

# 13. Scripts Folder

Digunakan untuk otomatisasi.

```
scripts/

pre_build.py

post_build.py

generate_build.py

release.py

flash.py
```

---

# 14. Build Automation

Build berjalan dengan urutan berikut.

```mermaid
flowchart TD

Build

-->

Generate BuildInfo

-->

Compile

-->

Static Analysis

-->

Unit Test

-->

Generate Firmware

-->

Generate Report
```

---

# 15. BuildInfo Generation

Sebelum compile.

Script menghasilkan:

```
BuildInfo.h
```

Berisi:

```cpp
constexpr uint16_t BUILD_NUMBER;

constexpr char BUILD_DATE[];

constexpr char BUILD_TIME[];

constexpr char GIT_HASH[];
```

File dibuat otomatis.

---

# 16. Version Structure

```
Version.h
```

```cpp
constexpr uint8_t VERSION_MAJOR = 1;
constexpr uint8_t VERSION_MINOR = 0;
constexpr uint8_t VERSION_PATCH = 0;
constexpr uint16_t VERSION_BUILD = 1;
```

Tidak boleh diubah manual selain saat release.

---

# 17. PlatformIO Configuration

Disarankan:

```ini
[env:nano]

platform = atmelavr

board = nanoatmega328

framework = arduino

monitor_speed = 115200

lib_ldf_mode = chain+

build_unflags =
    -std=gnu++11

build_flags =
    -std=gnu++17
    -Wall
    -Wextra
    -Werror
```

---

# 18. Library Dependency Rule

Dependency diperbolehkan:

```
Application

↓

Services

↓

Drivers

↓

HAL
```

Tidak boleh:

```
Driver

↓

Application
```

---

# 19. Header Dependency Rule

Header hanya boleh meng-include yang benar-benar diperlukan.

Gunakan:

- Forward Declaration
- const reference

Sebisa mungkin mengurangi compile time.

---

# 20. Common Module

Tambahkan folder:

```
lib/Common/
```

Berisi:

```
Types.h

Status.h

Event.h

Utilities.h

RingBuffer.h

BitHelper.h

MathHelper.h
```

Seluruh modul menggunakan tipe yang sama.

---

# 21. Event Folder

Disarankan membuat folder:

```
lib/Common/Event/
```

Berisi:

```
ButtonEvent.h

NotificationEvent.h

SystemEvent.h

EventQueue.h
```

---

# 22. Interface Folder

Untuk dependency injection.

```
lib/Common/Interfaces/

IDisplay.h

IRtc.h

IButton.h

IMode.h

INotification.h
```

Semua interface berada di satu lokasi.

---

# 23. HAL Structure

```
hal/

GPIO

I2C

Timer

EEPROM

Watchdog (Future)

Power (Future)
```

HAL harus independen terhadap aplikasi.

---

# 24. Production Folder

Direkomendasikan menambahkan:

```
production/

PCB/

Gerber/

BOM/

Firmware/

Release/

Manual/

Label/
```

Seluruh data produksi berada pada satu lokasi.

---

# 25. GitHub Structure

```
.github/

workflows/

issue_template/

pull_request_template.md
```

Workflow:

- Build
- Unit Test
- Static Analysis

---

# 26. Release Package

Release firmware terdiri dari:

```
Release/

Firmware.hex

Firmware.bin

Version.txt

ReleaseNote.pdf

CHANGELOG.md

Checksum.sha256
```

---

# 27. Recommended Workflow

```mermaid
graph TD

Feature

-->

Commit

-->

Pull Request

-->

Review

-->

Develop

-->

Release

-->

Main
```

---

# 28. Coding Ownership

| Folder | Owner |
|----------|-------|
| app | Application Developer |
| services | System Developer |
| drivers | Embedded Developer |
| hal | Embedded Developer |
| docs | Technical Writer / Developer |
| test | QA Engineer |

---

# 29. Production Checklist

- ☐ Folder Structure sesuai standar.
- ☐ Build berhasil.
- ☐ Unit Test lulus.
- ☐ Dokumentasi diperbarui.
- ☐ Version diperbarui.
- ☐ BuildInfo dihasilkan.
- ☐ Release Package lengkap.

---

# 30. Related Documents

- README.md
- 09_Firmware_Architecture.md
- 10_Coding_Standard.md
- 12_Testing_Checklist.md

---

# Implementation Notes

## Architecture Improvement 1 — Shared Interfaces

Seluruh interface dipindahkan ke satu lokasi:

```
lib/Common/Interfaces/
```

Contoh:

```
IMode.h

IDisplay.h

IRtc.h

INotification.h

IButton.h
```

Hal ini menghindari circular dependency.

---

## Architecture Improvement 2 — Shared Core Module

Semua utilitas umum dipusatkan pada:

```
lib/Common/
```

Berisi:

- RingBuffer
- Event
- Status
- Types
- Utilities

Tidak ada implementasi utilitas yang diduplikasi di setiap driver.

---

## Architecture Improvement 3 — BuildInfo Generator

Sebelum proses compile, script otomatis menghasilkan:

```
BuildInfo.h
```

Berisi:

- Build Number
- Build Date
- Build Time
- Git Commit
- Compiler Version (opsional)

Sehingga setiap firmware dapat diidentifikasi secara unik.

---

## Architecture Improvement 4 — Reusable Libraries

Display, Button, RTC, Notification, dan Scheduler dipisahkan menjadi library PlatformIO yang berdiri sendiri.

Keuntungan:

- Mudah diuji.
- Mudah digunakan ulang.
- Mempercepat pengembangan proyek berikutnya.

---

## Architecture Improvement 5 — Production Directory

Ditambahkan folder:

```
production/
```

yang berisi seluruh aset manufaktur:

- Firmware Release
- PCB
- Gerber
- BOM
- Label
- Manual
- QC Checklist

Ini memisahkan artefak produksi dari source code.

---

## Architecture Improvement 6 — Generated Files

Seluruh file hasil generate ditempatkan pada satu lokasi.

```
generated/

BuildInfo.h

SegmentFont.h

VersionString.h
```

Developer **tidak boleh** mengedit file dalam folder ini secara manual.

---

## Architecture Improvement 7 — Feature-Oriented Module

Untuk pengembangan jangka panjang, setiap fitur baru harus mengikuti pola berikut:

```
Feature/

include/

src/

test/

README.md
```

Sehingga fitur dapat dipindahkan menjadi library tanpa refactoring besar.

---

## Architecture Improvement 8 — Board Support Package (BSP)

Sebagai penyempurnaan arsitektur, saya menambahkan satu lapisan yang belum ada pada dokumen sebelumnya.

```
src/

bsp/

Board.cpp

Board.h
```

BSP bertanggung jawab terhadap:

- Inisialisasi seluruh hardware.
- Urutan startup.
- Pembuatan (instansiasi) seluruh driver.
- Penyediaan dependency ke Application.

Dengan adanya BSP:

```
main.cpp

↓

Board

↓

Services

↓

Drivers

↓

HAL
```

`main.cpp` menjadi sangat sederhana dan tidak mengetahui detail hardware.

Contoh:

```cpp
int main()
{
    Board board;

    board.begin();

    while (true)
    {
        board.run();
    }
}
```

Pendekatan ini memudahkan migrasi ke board lain tanpa mengubah logika aplikasi.

---

# Production Notes

- Struktur direktori merupakan bagian dari standar proyek dan tidak boleh diubah tanpa pembaruan dokumen arsitektur.
- Seluruh modul baru harus ditempatkan pada layer yang sesuai dan tidak boleh melanggar aturan dependency.
- Script otomatis (`scripts/`) harus dapat dijalankan pada Windows, Linux, dan macOS.
- File yang dihasilkan secara otomatis (`generated/`) tidak boleh diedit secara manual dan sebaiknya dikecualikan dari proses review kode.
- Seluruh library di folder `lib/` harus memiliki `README.md` dan `library.json` agar dapat digunakan kembali pada proyek PlatformIO lainnya.

---

**End of Document**
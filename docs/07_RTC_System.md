# 07 - RTC System

> Real Time Clock (RTC) System Specification for Operation Timer

**Document ID** : OT-DOC-007  
**Document Name** : RTC System  
**Project** : Operation Timer  
**Version** : 1.0.0  
**Status** : Draft  
**Last Update** : 2026-07-30

---

# Revision History

| Version | Date | Author | Description |
|----------|------------|----------------|------------------------------|
| 1.0.0 | 2026-07-30 | Development Team | Initial Document |

---

# 1. Purpose

Dokumen ini menjelaskan spesifikasi implementasi Real Time Clock (RTC) menggunakan modul **DS3231** pada proyek Operation Timer.

RTC digunakan sebagai sumber waktu utama (Master Time Reference) untuk seluruh sistem sehingga Clock, Stopwatch, Countdown, dan Scheduler memiliki referensi waktu yang sama.

---

# 2. Scope

Dokumen ini mencakup:

- RTC Hardware
- RTC Driver
- SQW Interrupt
- Time Synchronization
- Clock Mode
- Scheduler Integration
- Time Setting
- Error Handling
- Coding Rules

---

# 3. RTC Overview

Komponen RTC yang digunakan:

```
DS3231
```

Interface:

```
I²C
```

Supply:

```
5V
```

SQW Output:

```
1 Hz
```

Backup Battery:

```
CR2032
```

---

# 4. RTC Architecture

```mermaid
graph TD

DS3231

-->

RTC Driver

-->

Time Service

-->

Scheduler

Scheduler --> ClockMode

Scheduler --> StopwatchMode

Scheduler --> CountdownMode

ClockMode --> Display
```

RTC menjadi satu-satunya sumber waktu absolut.

---

# 5. Hardware Connection

| RTC Pin | Arduino Signal |
|----------|----------------|
| SDA | I2C_SDA |
| SCL | I2C_SCL |
| SQW | RTC_SQW |
| VCC | +5V |
| GND | GND |

---

# 6. SQW Configuration

RTC harus dikonfigurasi sebagai:

| Parameter | Value |
|------------|-------|
| Output | SQW |
| Frequency | 1 Hz |
| Mode | Open Drain |

Arduino menggunakan:

```
INPUT_PULLUP
```

SQW menghasilkan interrupt setiap satu detik.

---

# 7. RTC Driver Responsibilities

RTC Driver bertanggung jawab terhadap:

- Inisialisasi DS3231
- Membaca waktu
- Menulis waktu
- Sinkronisasi Scheduler
- Pemeriksaan komunikasi I²C

RTC Driver **tidak bertanggung jawab** terhadap:

- Display
- Stopwatch
- Countdown
- Button
- UI

---

# 8. Time Format

Seluruh firmware menggunakan format:

```
24 Hour
```

Representasi:

```
HH:MM:SS
```

Rentang:

| Field | Range |
|---------|--------|
| Hour | 00-23 |
| Minute | 00-59 |
| Second | 00-59 |

---

# 9. Time Data Structure

Disarankan struktur berikut:

```cpp
struct Time
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};
```

Seluruh parameter dikirim menggunakan:

```cpp
const Time&
```

untuk mengurangi penggunaan SRAM.

---

# 10. Scheduler Synchronization

Scheduler menggunakan referensi:

```text
RTC SQW

↓

1Hz Tick

↓

Scheduler

↓

Application
```

Dengan demikian:

- Clock akurat
- Stopwatch stabil
- Countdown stabil

---

# 11. System Tick

Setiap SQW menghasilkan event.

```mermaid
flowchart TD

SQW Interrupt

-->

Tick Flag

-->

Scheduler

-->

Mode Manager

-->

Display Update
```

ISR hanya mengatur flag.

Semua proses dilakukan di Main Loop.

---

# 12. Interrupt Rules

ISR hanya diperbolehkan:

- Set Tick Flag
- Exit

ISR **tidak boleh**:

- Membaca I²C
- Membaca RTC
- Mengakses Display
- Mengakses Button
- Menggunakan Serial
- Menggunakan delay()

---

# 13. RTC Read Policy

Pembacaan waktu dilakukan:

- Saat startup
- Setiap Tick 1 Hz
- Saat sinkronisasi waktu

RTC **tidak** dibaca terus-menerus.

Target:

```
1 Read / Second
```

---

# 14. RTC Write Policy

Penulisan RTC hanya dilakukan ketika:

- User menyimpan waktu baru
- Factory Calibration
- Initial Setup

Penulisan tidak boleh dilakukan setiap detik.

---

# 15. Clock Mode

Clock Mode selalu menggunakan data terbaru dari RTC.

Flow:

```mermaid
flowchart LR

RTC

-->

RTC Driver

-->

Clock Mode

-->

Display
```

---

# 16. RTC Initialization

Saat startup:

1. Inisialisasi I²C.
2. Deteksi DS3231.
3. Konfigurasi SQW 1Hz.
4. Baca waktu.
5. Sinkronkan Scheduler.

Jika gagal, masuk ke Error State.

---

# 17. Error Handling

Kemungkinan error:

- RTC tidak terdeteksi
- I²C timeout
- Data tidak valid
- Battery RTC habis

Penanganan:

- Retry komunikasi
- Gunakan data terakhir yang valid
- Aktifkan indikator error (future feature)

Firmware tidak boleh hang.

---

# 18. RTC Validation

Validasi data:

| Field | Valid |
|---------|-------|
| Hour | 0-23 |
| Minute | 0-59 |
| Second | 0-59 |

Jika data tidak valid:

- Abaikan data
- Gunakan data sebelumnya

---

# 19. API

Disarankan API:

```cpp
begin()

update()

readTime()

writeTime()

isConnected()

sync()

getTime()
```

---

# 20. Internal Class Structure

```text
RtcDriver
│
├── DS3231
├── Time
├── SQW Handler
└── I2C Interface
```

---

# 21. Memory Optimization

RTC Driver menggunakan:

- Static Allocation
- constexpr
- const
- Passing by Reference
- enum class

Tidak menggunakan:

- malloc()
- free()
- new
- delete()
- String

---

# 22. Communication Rules

RTC Driver hanya berkomunikasi melalui:

```
Wire Library
```

Seluruh komunikasi I²C harus:

- Timeout Protected
- Error Checked
- Non Blocking semaksimal mungkin

---

# 23. Coding Rules

RTC Driver wajib:

- Tidak menggunakan delay()
- Tidak menggunakan polling cepat
- Tidak melakukan pembacaan RTC di ISR
- Tidak mengakses Display
- Tidak mengakses Button
- Menggunakan konstanta `constexpr`
- Menggunakan parameter `const &` untuk object

---

# 24. Future Expansion

RTC Driver harus mendukung:

- Date
- Day of Week
- Temperature Sensor DS3231
- RTC Calibration
- DST (Daylight Saving Time)
- NTP Synchronization (Future Hardware)

Tanpa mengubah API utama.

---

# 25. Validation Checklist

RTC Driver dinyatakan lulus apabila:

- ☐ RTC terdeteksi.
- ☐ SQW bekerja 1Hz.
- ☐ Waktu terbaca benar.
- ☐ Waktu dapat disimpan.
- ☐ Scheduler sinkron.
- ☐ Clock akurat.
- ☐ Tidak ada blocking.
- ☐ Tidak membaca RTC di ISR.
- ☐ Recovery dari error berjalan.

---

# 26. Related Documents

- 02_Hardware_Architecture.md
- 03_Pin_Mapping.md
- 04_Display_Driver.md
- 06_Mode_Manager.md
- 09_Firmware_Architecture.md
- 10_Coding_Standard.md
- 12_Testing_Checklist.md

---

# Implementation Notes

## Time Source Hierarchy

Seluruh firmware menggunakan hierarki berikut.

```mermaid
graph TD

DS3231

-->

RTC Driver

-->

Time Service

-->

Scheduler

Scheduler --> Clock

Scheduler --> Stopwatch

Scheduler --> Countdown
```

Tidak ada modul lain yang membaca DS3231 secara langsung.

---

## Time Ownership

RTC Driver merupakan satu-satunya pemilik (owner) data waktu.

Mode lain memperoleh waktu melalui API RTC Driver.

Hal ini menghindari pembacaan I²C oleh beberapa modul secara bersamaan.

---

## Tick Synchronization

ISR SQW hanya melakukan:

```cpp
g_tick1Hz = true;
```

Main Loop:

```text
Scheduler

↓

RtcDriver.update()

↓

ModeManager.onTick1Hz()

↓

Display Update
```

Pendekatan ini menjaga ISR tetap sangat singkat.

---

## Recommended Time Structure

```cpp
struct Time
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};
```

Seluruh fungsi menerima parameter:

```cpp
const Time&
```

Sesuai standar optimasi memori proyek.

---

## I²C Access Policy

Seluruh akses Wire hanya dilakukan oleh `RtcDriver`.

Modul lain tidak diperbolehkan memanggil:

```cpp
Wire.beginTransmission()
Wire.requestFrom()
```

secara langsung.

---

# Production Notes

- Modul DS3231 yang digunakan harus merupakan versi dengan kristal internal (bukan DS1307).
- Baterai backup CR2032 harus dipasang sebelum proses kalibrasi waktu.
- Frekuensi SQW wajib diverifikasi sebesar **1 Hz** menggunakan osiloskop atau logic analyzer selama validasi hardware.
- Firmware release harus diuji minimal **24 jam** untuk memastikan sinkronisasi Clock, Stopwatch, dan Countdown tetap stabil terhadap RTC.
- Jika modul RTC diganti pada revisi hardware, driver harus tetap mempertahankan API publik agar kompatibilitas firmware terjaga.

---

**End of Document**
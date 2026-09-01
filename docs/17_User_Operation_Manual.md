Berikut isi `17_User_Operation_Manual.md`. Dokumen ini dibuat sebagai **panduan operator**, sehingga langkah pengoperasian dijelaskan dari unit dalam kondisi OFF sampai penggunaan Clock, Stopwatch, Countdown, Factory Mode, dan Diagnostic.

````document id="58321"
# 17_User_Operation_Manual.md

# Operation Timer
## User Operation Manual

**Project:** Operation Timer  
**Target:** Arduino Nano / ATmega328P  
**Framework:** PlatformIO + Arduino Framework  
**Document:** User Operation Manual  
**Version:** 1.0.0

---

# 1. Tujuan

Dokumen ini menjelaskan cara mengoperasikan Operation Timer dari kondisi awal sampai penggunaan seluruh fungsi utama.

Dokumen ini ditujukan untuk:

- operator
- tester
- technician
- production staff
- engineering

Dokumen ini juga digunakan sebagai referensi utama sebelum menjalankan:

```text
18_Manual_Test_Procedure.md
````

---

# 2. Kontrol Unit

Operation Timer menggunakan lima tombol utama:

| Tombol | Fungsi Umum                      |
| ------ | -------------------------------- |
| POWER  | Power / system control           |
| SELECT | Select / confirm / start / pause |
| NEXT   | Pindah mode / menu               |
| UP     | Naik / tambah nilai              |
| DOWN   | Turun / kurangi nilai            |

> Detail perilaku tombol mengikuti `05_Button_System.md` dan `13_UI_UX_Specification.md`.

---

# 3. Jenis Tekanan Tombol

Sistem membedakan beberapa jenis operasi tombol:

### Short Press

Tekan kemudian segera lepaskan.

```text
PRESS
  ↓
RELEASE
```

Digunakan untuk fungsi normal seperti:

* memilih menu
* pindah mode
* increment
* decrement
* confirm

### Long Press / Hold

Tekan dan tahan tombol selama waktu hold yang ditentukan oleh firmware.

```text
PRESS
  ↓
HOLD
  ↓
RELEASE
```

Digunakan untuk fungsi khusus yang membutuhkan proteksi terhadap accidental operation.

### Repeat

Tombol UP atau DOWN dapat menghasilkan repeat event ketika ditahan.

```text
PRESS
  ↓
HOLD
  ↓
REPEAT
  ↓
REPEAT
  ↓
RELEASE
```

Perilaku repeat mengikuti `05_Button_System.md`.

---

# 4. Kondisi Awal Unit

Sebelum melakukan operasi:

1. Pastikan unit sudah dirakit dengan benar.
2. Pastikan power supply sesuai spesifikasi.
3. Pastikan tidak ada kabel atau connector yang longgar.
4. Pastikan display terlihat jelas.
5. Pastikan tombol dapat ditekan dengan normal.

Kondisi awal:

```text
POWER OFF
```

---

# 5. Menyalakan Unit

## 5.1 Power ON

1. Pastikan unit dalam kondisi OFF.
2. Hubungkan power supply.
3. Aktifkan power supply.
4. Tunggu proses startup firmware.

Setelah boot selesai, unit masuk ke kondisi operasi normal.

Display harus menunjukkan Clock Mode sesuai konfigurasi default.

Contoh:

```text
12:34:56
```

LED dan buzzer mengikuti konfigurasi startup yang ditentukan firmware.

---

# 6. Pemeriksaan Setelah Power ON

Setelah unit menyala, periksa:

* [ ] display aktif
* [ ] seluruh digit normal
* [ ] tidak ada segment random
* [ ] tidak ada flicker abnormal
* [ ] LED berada pada state yang benar
* [ ] buzzer tidak berbunyi terus-menerus
* [ ] tombol merespons
* [ ] waktu RTC tampil normal

Jika semua normal, unit siap digunakan.

---

# 7. Mematikan Unit

Gunakan tombol:

```text
POWER
```

sesuai behavior yang didefinisikan pada UI/UX specification.

Jika firmware menggunakan software power-state, sistem harus berpindah ke kondisi aman.

Jika hardware menggunakan power switch eksternal, gunakan switch tersebut.

Setelah OFF:

* display OFF atau masuk kondisi sesuai desain
* buzzer OFF
* output tidak boleh berada pada kondisi berbahaya

---

# 8. Struktur Mode Utama

Operation Timer memiliki tiga mode utama:

```text
CLOCK
  ↓
STOPWATCH
  ↓
COUNTDOWN
  ↓
CLOCK
```

Perpindahan mode dilakukan menggunakan:

```text
NEXT
```

---

# 9. Clock Mode

Clock Mode digunakan untuk menampilkan waktu aktual dari RTC.

Format:

```text
HH:MM:SS
```

Format waktu:

```text
24-hour
```

Contoh:

```text
00:00:00
09:15:32
12:30:45
23:59:59
```

---

# 10. Menggunakan Clock Mode

Setelah unit dinyalakan:

```text
POWER ON
   ↓
CLOCK MODE
```

Display akan terus memperbarui waktu.

Contoh:

```text
12:30:58
12:30:59
12:31:00
12:31:01
```

Tidak diperlukan tombol untuk menjalankan Clock Mode.

RTC berjalan secara otomatis.

---

# 11. Memeriksa Clock

Untuk memastikan clock berjalan:

1. Catat waktu yang tampil.
2. Tunggu beberapa detik.
3. Perhatikan digit detik.
4. Pastikan detik bertambah secara normal.

Contoh:

```text
12:30:01
12:30:02
12:30:03
12:30:04
```

Jika waktu tidak berubah, masuk ke Diagnostic Mode untuk pemeriksaan RTC.

---

# 12. Pindah dari Clock ke Stopwatch

Dari Clock Mode:

```text
CLOCK
```

Tekan:

```text
NEXT
```

Unit berpindah ke:

```text
STOPWATCH
```

Display akan menunjukkan kondisi Stopwatch.

Contoh:

```text
00:00:00
```

---

# 13. Stopwatch Mode

Stopwatch digunakan untuk menghitung waktu yang telah berlalu.

State utama:

```text
STOPPED
RUNNING
PAUSED
```

Alur umum:

```text
STOPPED
   │
   │ START
   ▼
RUNNING
   │
   │ PAUSE
   ▼
PAUSED
   │
   │ RESUME
   ▼
RUNNING
```

---

# 14. Memulai Stopwatch

1. Pastikan unit berada pada Clock Mode.
2. Tekan `NEXT`.
3. Pastikan display menunjukkan Stopwatch.
4. Pastikan nilai awal:

```text
00:00:00
```

5. Tekan `SELECT`.

Stopwatch mulai berjalan.

Contoh:

```text
00:00:00
00:00:01
00:00:02
00:00:03
00:00:04
```

---

# 15. Menghentikan Sementara Stopwatch

Ketika Stopwatch sedang berjalan:

```text
RUNNING
```

Tekan:

```text
SELECT
```

Stopwatch masuk ke:

```text
PAUSED
```

Contoh:

```text
00:01:23
```

Display harus berhenti pada nilai tersebut.

---

# 16. Melanjutkan Stopwatch

Ketika Stopwatch dalam kondisi:

```text
PAUSED
```

Tekan:

```text
SELECT
```

Stopwatch kembali:

```text
RUNNING
```

Contoh:

```text
00:01:23
00:01:24
00:01:25
00:01:26
```

Nilai tidak boleh kembali ke zero.

---

# 17. Reset Stopwatch

Gunakan prosedur reset yang ditentukan oleh UI/UX specification.

Jika reset menggunakan long press:

```text
HOLD SELECT
```

atau kombinasi yang ditentukan firmware.

Setelah reset:

```text
00:00:00
```

> Jika implementasi final menggunakan tombol atau kombinasi berbeda, gunakan mapping aktual pada `13_UI_UX_Specification.md`.

---

# 18. Keluar dari Stopwatch

Ketika berada di Stopwatch Mode:

```text
STOPWATCH
```

Tekan:

```text
NEXT
```

Unit berpindah ke:

```text
COUNTDOWN
```

Status Stopwatch harus ditangani sesuai behavior Mode Manager.

---

# 19. Countdown Mode

Countdown digunakan untuk menghitung mundur dari nilai tertentu menuju:

```text
00:00:00
```

State utama:

```text
IDLE
RUNNING
PAUSED
COMPLETED
```

Alur:

```text
IDLE
  │
  │ START
  ▼
RUNNING
  │
  │ PAUSE
  ▼
PAUSED
  │
  │ RESUME
  ▼
RUNNING
  │
  │ REACH ZERO
  ▼
COMPLETED
```

---

# 20. Masuk Countdown Mode

Dari Stopwatch:

```text
STOPWATCH
```

Tekan:

```text
NEXT
```

Unit masuk:

```text
COUNTDOWN
```

Contoh display:

```text
00:05:00
```

atau nilai terakhir yang tersimpan sesuai behavior firmware.

---

# 21. Menyiapkan Nilai Countdown

Pilih bagian waktu yang akan diubah menggunakan:

```text
SELECT
```

Kemudian gunakan:

```text
UP
DOWN
```

untuk mengubah nilai.

Contoh:

```text
00:05:00
```

diubah menjadi:

```text
00:10:00
```

Dengan menggunakan:

```text
UP
```

Nilai bertambah sesuai increment yang ditentukan firmware.

---

# 22. Mengurangi Nilai Countdown

Gunakan:

```text
DOWN
```

untuk mengurangi nilai.

Contoh:

```text
00:10:00
```

menjadi:

```text
00:09:00
```

Nilai tidak boleh melewati batas minimum.

---

# 23. Mengubah Nilai dengan Hold

Untuk perubahan nilai yang cepat:

1. Tekan dan tahan `UP`.
2. Firmware menghasilkan repeat event.
3. Nilai bertambah secara otomatis.
4. Lepaskan tombol ketika nilai yang diinginkan tercapai.

Hal yang sama berlaku untuk:

```text
DOWN
```

untuk mengurangi nilai.

---

# 24. Memulai Countdown

Setelah nilai countdown ditentukan:

Contoh:

```text
00:00:10
```

Tekan tombol start/confirm sesuai UI specification.

Pada implementasi default:

```text
SELECT
```

digunakan untuk menjalankan countdown dari kondisi siap.

Countdown mulai:

```text
00:00:10
00:00:09
00:00:08
00:00:07
...
00:00:01
00:00:00
```

---

# 25. Pause Countdown

Ketika Countdown sedang berjalan:

```text
RUNNING
```

Tekan:

```text
SELECT
```

Countdown masuk:

```text
PAUSED
```

Contoh:

```text
00:03:25
```

Display berhenti pada nilai tersebut.

---

# 26. Resume Countdown

Ketika Countdown dalam kondisi:

```text
PAUSED
```

Tekan:

```text
SELECT
```

Countdown melanjutkan:

```text
00:03:25
00:03:24
00:03:23
...
```

---

# 27. Countdown Completion

Ketika Countdown mencapai:

```text
00:00:00
```

timer berhenti.

System menghasilkan notification sesuai specification.

Notification dapat berupa:

```text
BUZZER
LED
DISPLAY STATE
```

Contoh alur:

```text
00:00:02
    ↓
00:00:01
    ↓
00:00:00
    ↓
NOTIFICATION
```

Buzzer harus berhenti setelah pattern notification selesai.

---

# 28. Setelah Countdown Selesai

Setelah completion:

```text
00:00:00
```

unit tidak boleh melanjutkan ke:

```text
-00:00:01
```

atau kembali ke nilai maksimum.

Operator dapat melakukan reset atau melakukan setup countdown berikutnya sesuai UI specification.

---

# 29. Pindah Mode dari Countdown

Tekan:

```text
NEXT
```

Unit kembali ke:

```text
CLOCK
```

Sehingga siklus mode menjadi:

```text
CLOCK
  ↓ NEXT
STOPWATCH
  ↓ NEXT
COUNTDOWN
  ↓ NEXT
CLOCK
```

---

# 30. Ringkasan Operasi Mode

| Kondisi           | Tombol | Hasil     |
| ----------------- | ------ | --------- |
| Clock             | NEXT   | Stopwatch |
| Stopwatch         | NEXT   | Countdown |
| Countdown         | NEXT   | Clock     |
| Stopwatch stopped | SELECT | Start     |
| Stopwatch running | SELECT | Pause     |
| Stopwatch paused  | SELECT | Resume    |
| Countdown ready   | SELECT | Start     |
| Countdown running | SELECT | Pause     |
| Countdown paused  | SELECT | Resume    |
| Setting value     | UP     | Increase  |
| Setting value     | DOWN   | Decrease  |

> Jika implementasi firmware final menggunakan kombinasi tombol berbeda, tabel ini harus disesuaikan dengan `13_UI_UX_Specification.md`.

---

# 31. Contoh Penggunaan Stopwatch

Berikut contoh penggunaan nyata.

Tujuan:

```text
Mengukur waktu proses selama 30 detik.
```

Langkah:

```text
1. POWER ON
2. Tunggu Clock Mode
3. Tekan NEXT
4. Pastikan Stopwatch Mode
5. Pastikan display 00:00:00
6. Tekan SELECT
7. Lakukan aktivitas yang akan diukur
8. Tunggu sekitar 30 detik
9. Tekan SELECT
10. Stopwatch berhenti
11. Baca hasil
```

Contoh hasil:

```text
00:00:31
```

---

# 32. Contoh Stopwatch Pause / Resume

1. Masuk Stopwatch.
2. Tekan `SELECT`.
3. Tunggu sampai:

```text
00:00:10
```

4. Tekan `SELECT`.
5. Stopwatch pause.
6. Tunggu 5 detik.
7. Display harus tetap:

```text
00:00:10
```

8. Tekan `SELECT`.
9. Stopwatch resume.

Hasil berikutnya:

```text
00:00:11
00:00:12
...
```

---

# 33. Contoh Penggunaan Countdown

Tujuan:

```text
Countdown 10 detik.
```

Langkah:

```text
1. POWER ON
2. Tekan NEXT
3. Tekan NEXT
4. Masuk Countdown
5. Set nilai 00:00:10
6. Tekan SELECT
7. Countdown berjalan
8. Tunggu sampai 00:00:00
9. Periksa buzzer
10. Periksa LED
```

Expected:

```text
00:00:10
00:00:09
00:00:08
...
00:00:01
00:00:00
```

Kemudian:

```text
BUZZER / NOTIFICATION
```

---

# 34. Menguji UP dan DOWN

Contoh:

```text
Nilai awal:
00:01:00
```

Tekan:

```text
UP
```

Expected:

```text
00:02:00
```

Tekan:

```text
DOWN
```

Expected:

```text
00:01:00
```

Untuk pengujian repeat:

1. Tekan dan tahan `UP`.
2. Amati nilai.
3. Nilai harus meningkat secara bertahap.
4. Lepaskan tombol.
5. Nilai berhenti berubah.

---

# 35. Factory Mode

Factory Mode digunakan untuk pemeriksaan hardware dan produksi.

Factory Mode dapat digunakan untuk menguji:

```text
DISPLAY
BUTTON
LED
BUZZER
RTC
```

Factory Mode bukan mode penggunaan normal.

---

# 36. Masuk Factory Mode

Gunakan kombinasi tombol / prosedur Factory Mode yang ditentukan oleh:

```text
22_Factory_Mode.md
13_UI_UX_Specification.md
```

Setelah berhasil masuk, display menunjukkan indikasi Factory Mode.

Contoh:

```text
FACTORY
```

atau indikator sesuai implementasi display.

---

# 37. Display Test

Di Factory Mode:

1. Pilih Display Test.
2. Jalankan test.
3. Amati seluruh digit.
4. Amati seluruh segment.

Test harus dapat memverifikasi:

```text
DIGIT 1
DIGIT 2
DIGIT 3
DIGIT 4
DIGIT 5
DIGIT 6
```

dan segment:

```text
A
B
C
D
E
F
G
```

---

# 38. Button Test

Di Factory Mode:

1. Pilih Button Test.
2. Tekan `POWER`.
3. Tekan `SELECT`.
4. Tekan `NEXT`.
5. Tekan `UP`.
6. Tekan `DOWN`.

Display / diagnostic harus menunjukkan button event yang diterima.

Contoh:

```text
POWER
SELECT
NEXT
UP
DOWN
```

---

# 39. LED Test

Di Factory Mode:

1. Pilih LED Test.
2. Aktifkan LED.
3. Periksa LED secara visual.
4. Nonaktifkan LED.

Expected:

```text
ON
OFF
```

LED tidak boleh menyala redup atau berkedip tanpa perintah.

---

# 40. Buzzer Test

Di Factory Mode:

1. Pilih Buzzer Test.
2. Jalankan test.
3. Pastikan buzzer berbunyi.
4. Pastikan buzzer berhenti setelah test.

Expected:

```text
BEEP
```

Tidak boleh terjadi:

```text
BUZZER STUCK ON
```

---

# 41. RTC Test

Di Factory Mode / Diagnostic Mode:

1. Pilih RTC Test.
2. Periksa apakah RTC terdeteksi.
3. Periksa waktu.
4. Tunggu beberapa detik.
5. Pastikan waktu bertambah.

Contoh:

```text
14:30:00
14:30:01
14:30:02
```

---

# 42. Diagnostic Mode

Diagnostic Mode digunakan untuk pemeriksaan kondisi sistem.

Informasi yang dapat diperiksa meliputi:

```text
Firmware Version
Hardware Revision
RTC Status
Display Status
Button Status
LED Status
Buzzer Status
```

Gunakan:

```text
23_Diagnostic_System.md
```

sebagai referensi detail.

---

# 43. Melihat Firmware Version

Masuk Diagnostic Mode.

Cari informasi:

```text
Firmware Version
```

Version menggunakan format:

```text
MAJOR.MINOR.PATCH
```

dengan build identifier sesuai `16_Firmware_Versioning.md`.

Contoh:

```text
1.2.3
Build 104
```

---

# 44. Kondisi Error

Jika unit menunjukkan kondisi tidak normal:

1. Jangan langsung melakukan reset berulang.
2. Catat kondisi display.
3. Catat LED.
4. Catat bunyi buzzer.
5. Catat tombol terakhir yang ditekan.
6. Masuk Diagnostic Mode jika memungkinkan.
7. Catat Firmware Version.
8. Catat Hardware Revision.
9. Lakukan pemeriksaan sesuai `18_Manual_Test_Procedure.md`.

---

# 45. Prosedur Reset Operasional

Jika aplikasi berada pada kondisi yang tidak diinginkan:

1. Pastikan tidak sedang melakukan operasi kritis.
2. Gunakan reset sesuai prosedur hardware/software.
3. Tunggu boot selesai.
4. Pastikan Clock Mode kembali normal.
5. Periksa RTC.
6. Ulangi operasi sebelumnya.

Reset tidak boleh digunakan untuk menutupi kegagalan fungsi.

---

# 46. Quick Start

Untuk operator yang sudah familiar dengan unit:

```text
POWER ON
   ↓
CLOCK
   │
   ├── NEXT → STOPWATCH
   │             │
   │             └── SELECT → START / PAUSE / RESUME
   │
   └── NEXT → COUNTDOWN
                 │
                 ├── UP / DOWN → SET
                 │
                 └── SELECT → START / PAUSE / RESUME
```

---

# 47. Quick Stopwatch Test

```text
1. POWER ON
2. NEXT
3. Pastikan STOPWATCH
4. SELECT
5. Tunggu
6. SELECT
7. Baca waktu
```

Expected:

```text
00:00:00
↓
00:00:01
↓
00:00:02
↓
...
```

---

# 48. Quick Countdown Test

```text
1. POWER ON
2. NEXT
3. NEXT
4. Pastikan COUNTDOWN
5. Set nilai
6. SELECT
7. Tunggu sampai zero
8. Periksa notification
```

Expected:

```text
00:00:03
00:00:02
00:00:01
00:00:00
BEEP
```

---

# 49. Quick Hardware Test

Urutan cepat:

```text
POWER ON
   ↓
DISPLAY
   ↓
BUTTON
   ↓
LED
   ↓
BUZZER
   ↓
RTC
   ↓
CLOCK
   ↓
STOPWATCH
   ↓
COUNTDOWN
```

Jika semua normal, lanjutkan ke:

```text
18_Manual_Test_Procedure.md
```

---

# 50. Operator Checklist

## Startup

* [ ] Power supply ON
* [ ] Unit boot
* [ ] Display aktif
* [ ] Clock tampil
* [ ] RTC berjalan

## Clock

* [ ] Waktu berjalan
* [ ] Format HH:MM:SS benar
* [ ] 24-hour format benar

## Stopwatch

* [ ] Masuk Stopwatch
* [ ] Start
* [ ] Pause
* [ ] Resume
* [ ] Reset

## Countdown

* [ ] Masuk Countdown
* [ ] Set value
* [ ] Start
* [ ] Pause
* [ ] Resume
* [ ] Reach zero
* [ ] Buzzer notification

## Hardware

* [ ] Display
* [ ] Buttons
* [ ] LED
* [ ] Buzzer
* [ ] RTC

## Diagnostic

* [ ] Firmware Version
* [ ] Hardware Revision
* [ ] RTC Status
* [ ] Hardware Status

---

# 51. Hubungan dengan Dokumen Testing

Setelah memahami prosedur operasi dalam dokumen ini, gunakan:

```text
18_Manual_Test_Procedure.md
```

untuk pengujian lengkap.

Perbedaan kedua dokumen:

| Dokumen                     | Tujuan                  |
| --------------------------- | ----------------------- |
| 17_User_Operation_Manual.md | Cara menggunakan unit   |
| 18_Manual_Test_Procedure.md | Cara memverifikasi unit |

---

# 52. Prinsip Pengoperasian

Operator harus selalu:

1. menekan tombol sesuai fungsi
2. menunggu response display
3. memastikan state berubah sesuai expected behavior
4. tidak menekan beberapa tombol secara bersamaan kecuali memang didefinisikan
5. tidak melakukan power cycle berulang tanpa alasan
6. mencatat kondisi abnormal
7. menggunakan Diagnostic Mode untuk troubleshooting

---

# 53. Catatan Implementasi

Dokumen ini merupakan panduan operasional tingkat pengguna.

Detail implementasi software tetap mengikuti:

```text
05_Button_System.md
06_Mode_Manager.md
09_Firmware_Architecture.md
13_UI_UX_Specification.md
22_Factory_Mode.md
23_Diagnostic_System.md
```

Jika terdapat perbedaan antara panduan ini dan implementasi firmware final, maka:

```text
13_UI_UX_Specification.md
```

menjadi referensi utama untuk behavior UI.

Setiap perubahan behavior UI harus diikuti dengan update terhadap dokumen ini.

---

# END OF DOCUMENT

```

Dokumen `17_User_Operation_Manual.md` ini sekarang menjadi **manual operator**, sedangkan `18_Manual_Test_Procedure.md` menjadi **dokumen QA/testing**. Dengan pemisahan ini, tester bisa mengikuti `17` terlebih dahulu untuk memahami cara memakai unit, kemudian menjalankan checklist `18` tanpa harus menebak urutan tombol.
```

# GPIO HAL

## Tujuan

GPIO HAL menyediakan abstraksi akses GPIO untuk firmware Operation Timer. Semua modul lain harus menggunakan HAL ini untuk mengakses pin hardware.

## Pin Mapping

- Input: RTC SQW, power/select/next/up/down buttons
- Output: buzzer, display OE, shift register latch/data/clock, power LED

## API

- `begin()` menginisialisasi seluruh pin sesuai role
- `pinModeInput()` dan `pinModeOutput()` mengatur arah pin
- `read()` membaca nilai pin
- `write()` menulis state logika
- `writeActiveLow()` menulis logika aktif-low untuk LED dan buzzer

## Dependency

- Menggunakan `common/Status.h`
- Tidak memiliki dependency ke driver lain

## Testing

Test dasar tersedia di `test/hal/gpio/gpio_test.cpp`.

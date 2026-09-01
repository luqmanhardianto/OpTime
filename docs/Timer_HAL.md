# Timer HAL

## Tujuan

Timer HAL menyediakan sumber waktu dasar untuk firmware Operation Timer. Modul ini mengabstraksikan akses ke sistem timer AVR dan memudahkan integrasi scheduler, display, dan timing lainnya.

## Timer Allocation

- Timer2 digunakan untuk tick sistem 1 ms
- Timer1 reserved untuk display multiplex
- Timer0 tetap tidak digunakan agar framework Arduino tidak terganggu

## API

- `begin()` menginisialisasi timer dan interrupt
- `millis()` mengembalikan uptime dalam milidetik
- `micros()` mengembalikan counter mikrodetik
- `attachTickCallback()` mendaftarkan callback untuk tick sistem

## Memory Usage

- Tidak menggunakan dynamic allocation
- Callback disimpan sebagai pointer fungsi statis
- Variabel ISR memakai `volatile`

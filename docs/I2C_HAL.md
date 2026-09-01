# I2C HAL

## Tujuan

I2C HAL menyediakan abstraksi komunikasi I2C untuk firmware Operation Timer. Modul ini digunakan oleh driver RTC dan siap diperluas untuk perangkat lain.

## Pin Mapping

- SDA: A4
- SCL: A5

## DS3231

- Address: 0x68
- Register example: 0x0E

## API

- `begin()` menginisialisasi bus dan mengatur clock 400 kHz
- `write()` mengirim data ke device
- `read()` membaca data dari device
- `writeRegister()` menulis register
- `readRegister()` membaca register

## Error Handling

- `StatusCode::OK` untuk sukses
- `StatusCode::ERROR` untuk no ACK / bus error
- `StatusCode::INVALID_PARAMETER` untuk parameter tidak valid

## Memory Usage

- Tidak menggunakan dynamic allocation
- Buffer terbatas di level fungsi secara statik

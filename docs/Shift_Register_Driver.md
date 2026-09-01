# Shift Register Driver

## Tujuan

Driver ini mengontrol dua 74HC595 sebagai chain untuk display 7-segment. Driver ini memisahkan logika display dari hardware dan siap dipanggil dari ISR atau loop utama.

## Wiring

- DATA: D11
- CLOCK: D13
- LATCH: D10
- OE: D9

## Sequence

1. Kirim digit data terlebih dahulu
2. Lalu kirim segment data
3. Panggil `latch()` setelah 16 bit terkirim
4. `setOutputEnable(true)` mengaktifkan output (OE LOW)

## Memory Usage

- Tidak menggunakan dynamic allocation
- Operasi dilakukan secara langsung ke pin hardware

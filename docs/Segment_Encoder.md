# Segment Encoder

## Tujuan

Segment Encoder mengubah digit angka dan karakter khusus menjadi pola byte 7-segment yang sesuai dengan hardware common-anode.

## Mapping

- Bit0 = E
- Bit1 = D
- Bit2 = C
- Bit3 = G
- Bit4 = F
- Bit5 = A
- Bit6 = B
- Bit7 = reserved

## Logic

- Common anode: segment aktif LOW
- Pola diambil dari tabel flash (`PROGMEM`) sehingga RAM tetap kecil

## API

- `encodeDigit(uint8_t digit)`
- `encodeChar(SegmentChar character)`
- `blank()`

## Memory Usage

- Lookup table tersimpan di flash
- Tidak ada alokasi dynamic

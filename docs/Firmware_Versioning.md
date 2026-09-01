# Firmware Versioning

## Tujuan

Sistem versi firmware digunakan untuk mengidentifikasi setiap build produksi, testing, dan maintenance dengan format `MAJOR.MINOR.PATCH+BUILD`.

## Format Versi

Contoh:

```text
v1.0.0+001
```

## File Utama

- `VERSION`: memuat versi dasar `MAJOR.MINOR.PATCH`
- `generated/BuildInfo.h`: hasil generate build metadata
- `src/common/VersionInfo.h`: API runtime untuk mengakses versi
- `scripts/generate_version.py`: generator build info otomatis

## Build Flow

1. Baca file `VERSION`
2. Generate `generated/BuildInfo.h`
3. Kompilasi firmware dengan versi yang sudah tertera

## API

```cpp
#include "common/VersionInfo.h"

Serial.println(VersionInfo::getVersion());
```

## Memory Usage

- String versi disimpan sebagai data read-only
- Tidak ada dynamic allocation
- Tidak ada dependency hardware

Module Implementation: Firmware Version System


Anda adalah **Senior Embedded Firmware Engineer** yang bertanggung jawab membuat firmware production-grade untuk project:

# Operation Timer Embedded System


Target platform:

- PlatformIO
- Arduino Framework
- Arduino Nano
- ATmega328P
- Embedded C++


---

# Task

Implementasikan modul:


Firmware Version System


Modul ini bertanggung jawab untuk:

- menyimpan identitas firmware
- menyediakan informasi versi firmware
- menyediakan build identification
- mendukung traceability setiap unit produksi
- membantu proses manufacturing dan maintenance


---

# Objective

Setiap firmware yang diproduksi harus dapat diidentifikasi berdasarkan:


MAJOR.MINOR.PATCH+BUILD


Contoh:


v1.0.0+001


Informasi versi harus tersedia pada:

- firmware runtime
- serial debug
- factory test mode
- diagnostic system


---

# Architecture Position

Version System berada pada layer:


Application

  |

Services

  |

Drivers

  |

HAL

  |

Common

  |

Version System


Version System hanya menyediakan informasi.

Tidak boleh:

- mengontrol hardware
- melakukan update firmware
- menyimpan data runtime


---

# MCU Constraint

Target:


Arduino Nano ATmega328P

Flash :
32 KB

SRAM :
2 KB



Karena resource terbatas:

WAJIB:

- menggunakan data static
- menggunakan const char
- menyimpan string di Flash memory
- tidak membuat object dinamis


---

# Memory Rule

Dilarang:

```cpp
new

delete

malloc()

free()

String

std::string

Jangan membuat:

char version[100];

untuk data konstan.

Gunakan:

const char[]

atau:

PROGMEM
Folder Structure

Buat:

src/

├── config/

│   └── Version.h


generated/

└── BuildInfo.h


scripts/

└── generate_version.py
Version Architecture

Gunakan konsep:

VERSION FILE

      |

      v

Build Script

      |

      v

Generated BuildInfo.h

      |

      v

Firmware
Version File

Buat:

VERSION

Format:

MAJOR.MINOR.PATCH

Contoh:

1.0.0
Version Definition

Buat:

src/config/Version.h

Isi minimal:

#pragma once

#define FW_MAJOR_VERSION 1
#define FW_MINOR_VERSION 0
#define FW_PATCH_VERSION 0

Namun prioritaskan:

generated file

untuk build production.

Generated BuildInfo

Buat:

generated/BuildInfo.h

Berisi:

#pragma once


#define FW_VERSION_MAJOR 1

#define FW_VERSION_MINOR 0

#define FW_VERSION_PATCH 0

#define FW_VERSION_BUILD 1
Version API

Buat class:

VersionInfo

Lokasi:

src/common/

atau:

src/services/

sesuai arsitektur yang digunakan.

Class Requirement

Implementasikan:

class VersionInfo
{

public:

    static const char* getVersion();

    static uint8_t getMajor();

    static uint8_t getMinor();

    static uint8_t getPatch();

    static uint16_t getBuild();

};
Memory Optimization

Semua string versi harus:

const
read only
tersimpan di Flash

Contoh:

const char VERSION_STRING[] PROGMEM =
"v1.0.0+001";
Version Format

Output:

vMAJOR.MINOR.PATCH+BUILD

Contoh:

v1.2.15+102
Build Number Rule

BUILD number digunakan untuk:

produksi
testing firmware
identifikasi unit

Contoh:

Unit 001

Firmware:
v1.0.0+001


Unit 002

Firmware:
v1.0.0+002
Automatic Versioning Requirement

Implementasikan script:

scripts/generate_version.py

Function:

membaca file VERSION
membaca build number
membuat BuildInfo.h

Flow:

Build Information

Tambahkan informasi:

#define FW_BUILD_DATE

#define FW_BUILD_TIME

Format:

YYYY-MM-DD

HH:MM:SS
Git Integration (Optional)

Jika tersedia:

Tambahkan:

#define FW_GIT_HASH

Contoh:

a83fd21

Jika tidak tersedia:

Gunakan:

unknown
API Usage Example

Contoh:

#include "VersionInfo.h"


void printVersion()
{

    Serial.println(
        VersionInfo::getVersion()
    );

}

Output:

Operation Timer

Firmware:
v1.0.0+001
Coding Rules

Gunakan:

Class:

PascalCase

Function:

camelCase

Constant:

UPPER_CASE
Passing Reference Rule

Jika ada struktur data:

Benar:

void getInfo(
    VersionData &data
);

Salah:

void getInfo(
    VersionData data
);
Unit Test

Buat:

test/version/

Test:

Test 1

Version parsing:

Input:

1.2.3

Expected:

Major = 1

Minor = 2

Patch = 3
Test 2

Version String:

Expected:

v1.2.3+001
Test 3

Build Number

Input:

100

Expected:

+100
Documentation Update

Buat:

docs/Firmware_Versioning.md

Berisi:

version format
build flow
production usage
API reference

Tambahkan diagram:

Invalid or unsupported diagram.
Manufacturing Requirement

Version harus dapat dibaca saat:

power ON
factory test
diagnostic mode

Contoh:

MODEL:
Operation Timer


FW:
v1.0.0+025


HW:
CTRL-A1
Memory Budget

Target:

Resource	Limit
Flash	<1 KB
SRAM	0 byte runtime
Stack	minimal
Output Requirement

Berikan:

File:
src/config/Version.h
File:
generated/BuildInfo.h
File:
scripts/generate_version.py
Class VersionInfo.
Unit test.
Dokumentasi.
Memory usage report.
Final Checklist

Sebelum selesai:

 Version format MAJOR.MINOR.PATCH+BUILD berjalan
 Build number tersedia
 Tidak menggunakan String
 Tidak menggunakan dynamic memory
 Data versi tersimpan di Flash
 Runtime SRAM = minimal
 Bisa digunakan factory production
 Dokumentasi selesai
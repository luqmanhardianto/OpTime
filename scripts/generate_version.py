#!/usr/bin/env python3
import os
import sys
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
version_file = ROOT / 'VERSION'
build_info_file = ROOT / 'generated' / 'BuildInfo.h'

if not version_file.exists():
    raise SystemExit('VERSION file not found')

version = version_file.read_text(encoding='utf-8').strip()
parts = version.split('.')
if len(parts) != 3:
    raise SystemExit('VERSION must follow MAJOR.MINOR.PATCH')

major, minor, patch = [int(p) for p in parts]

build_number = 1
if os.environ.get('FW_BUILD_NUMBER'):
    build_number = int(os.environ['FW_BUILD_NUMBER'])

now_utc = datetime.now(timezone.utc)
build_date = now_utc.strftime('%Y-%m-%d')
build_time = now_utc.strftime('%H:%M:%S')
git_hash = os.popen('git rev-parse --short HEAD 2>/dev/null').read().strip() or 'unknown'

content = f'''#pragma once

#define FW_VERSION_MAJOR {major}
#define FW_VERSION_MINOR {minor}
#define FW_VERSION_PATCH {patch}
#define FW_VERSION_BUILD {build_number}
#define FW_BUILD_DATE "{build_date}"
#define FW_BUILD_TIME "{build_time}"
#define FW_GIT_HASH "{git_hash}"
'''

build_info_file.parent.mkdir(parents=True, exist_ok=True)
build_info_file.write_text(content, encoding='utf-8')
print(f'Generated {build_info_file}')

#!/usr/bin/env bash
set -euo pipefail

# Полный цикл: компиляция + merged bin для M5Stack Stick S3 (ESP32-S3)
#
# Использование:
#   ./make_bin.sh <FQBN> [BUILD_DIR]
#
# Пример FQBN (может отличаться в твоей установке):
#   esp32:esp32:m5stack_stickc_plus2

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <FQBN> [BUILD_DIR]"
  exit 1
fi

if ! command -v arduino-cli >/dev/null 2>&1; then
  echo "arduino-cli not found. Install it first."
  exit 1
fi

if ! command -v esptool.py >/dev/null 2>&1; then
  echo "esptool.py not found. Install it first."
  exit 1
fi

FQBN="$1"
BUILD_DIR="${2:-build}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKETCH="${SCRIPT_DIR}/main.ino"

mkdir -p "$BUILD_DIR"

echo "Compiling sketch with arduino-cli..."
arduino-cli compile \
  --fqbn "$FQBN" \
  --output-dir "$BUILD_DIR" \
  "$SKETCH"

# Типичные имена, которые arduino-cli кладет в output-dir.
BOOTLOADER_BIN="${BUILD_DIR}/bootloader.bin"
PARTITIONS_BIN="${BUILD_DIR}/partitions.bin"

# firmware-файл может называться как скетч: main.ino.bin
if [[ -f "${BUILD_DIR}/main.ino.bin" ]]; then
  FIRMWARE_BIN="${BUILD_DIR}/main.ino.bin"
elif [[ -f "${BUILD_DIR}/firmware.bin" ]]; then
  FIRMWARE_BIN="${BUILD_DIR}/firmware.bin"
else
  echo "Firmware bin not found in ${BUILD_DIR} (expected main.ino.bin or firmware.bin)"
  exit 1
fi

OUT_MERGED_BIN="${BUILD_DIR}/m5stick_s3_clock_merged.bin"

echo "Merging binaries into ${OUT_MERGED_BIN}..."
"${SCRIPT_DIR}/merge_bin.sh" "$BOOTLOADER_BIN" "$PARTITIONS_BIN" "$FIRMWARE_BIN" "$OUT_MERGED_BIN"

echo
echo "Ready: ${OUT_MERGED_BIN}"
echo "Это и есть один bin-файл для прошивки с телефона."

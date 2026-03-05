#!/usr/bin/env bash
set -euo pipefail

# Собирает единый merged.bin для ESP32-S3.
# Такой bin удобен для прошивки с телефона (один файл, адрес 0x0).
#
# Использование:
#   ./merge_bin.sh build/bootloader.bin build/partitions.bin build/firmware.bin build/m5stick_s3_clock_merged.bin

if [[ $# -ne 4 ]]; then
  echo "Usage: $0 <BOOTLOADER_BIN> <PARTITIONS_BIN> <FIRMWARE_BIN> <OUT_MERGED_BIN>"
  exit 1
fi

BOOTLOADER_BIN="$1"
PARTITIONS_BIN="$2"
FIRMWARE_BIN="$3"
OUT_MERGED_BIN="$4"

for f in "$BOOTLOADER_BIN" "$PARTITIONS_BIN" "$FIRMWARE_BIN"; do
  if [[ ! -f "$f" ]]; then
    echo "File not found: $f"
    exit 1
  fi
done

mkdir -p "$(dirname "$OUT_MERGED_BIN")"

echo "Creating merged binary: ${OUT_MERGED_BIN}"
esptool.py \
  --chip esp32s3 \
  merge_bin -o "$OUT_MERGED_BIN" \
  0x0000 "$BOOTLOADER_BIN" \
  0x8000 "$PARTITIONS_BIN" \
  0x10000 "$FIRMWARE_BIN"

echo "Merged bin ready: ${OUT_MERGED_BIN}"

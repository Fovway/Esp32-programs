#!/usr/bin/env bash
set -euo pipefail

# Прошивка M5Stick S3 (ESP32-S3) одним merged bin.
# Удобно для случаев, когда у тебя один .bin файл.
#
# Использование:
#   ./flash_esp32.sh /dev/ttyACM0 build/m5stick_s3_clock_merged.bin

if [[ $# -ne 2 ]]; then
  echo "Usage: $0 <PORT> <MERGED_BIN>"
  exit 1
fi

PORT="$1"
MERGED_BIN="$2"
BAUD="921600"

if [[ ! -f "$MERGED_BIN" ]]; then
  echo "File not found: $MERGED_BIN"
  exit 1
fi

echo "Flashing merged bin to ESP32-S3 on ${PORT}..."
esptool.py \
  --chip esp32s3 \
  --port "$PORT" \
  --baud "$BAUD" \
  --before default_reset \
  --after hard_reset \
  write_flash -z \
  0x0 "$MERGED_BIN"

echo "Done."

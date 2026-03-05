# firmware-02-clock-oled-m5stick-s3

Новая версия прошивки часов для **M5Stack Stick S3 (ESP32-S3)**.

## Что внутри

- `main.ino` — скетч часов с синхронизацией времени через NTP и выводом времени/даты на экран.
- `merge_bin.sh` — собирает **один** файл `merged.bin` из `bootloader.bin + partitions.bin + firmware.bin`.
- `make_bin.sh` — автоматизирует: **compile + merged.bin**.
- `flash_esp32.sh` — прошивает ESP32-S3 **одним merged bin** (адрес `0x0`).

## Зависимости

- библиотека `M5Unified` (для сборки скетча)
- `arduino-cli` (удобно для сборки бинарников)
- `esptool.py` (для merge и прошивки)

## Как сделать bin файл (коротко)

### Вариант 1 — одной командой (рекомендуется)

```bash
./make_bin.sh <FQBN> [BUILD_DIR]
```

Пример:

```bash
./make_bin.sh esp32:esp32:m5stack_stickc_plus2 build
```

После этого получишь готовый файл для телефона:

- `build/m5stick_s3_clock_merged.bin`

### Вариант 2 — вручную

1) Собери скетч и получи:

- `bootloader.bin`
- `partitions.bin`
- `main.ino.bin` (или `firmware.bin`)

2) Собери единый bin:

```bash
./merge_bin.sh build/bootloader.bin build/partitions.bin build/main.ino.bin build/m5stick_s3_clock_merged.bin
```

## Прошивка merged.bin

```bash
./flash_esp32.sh /dev/ttyACM0 build/m5stick_s3_clock_merged.bin
```

Если порт другой — замени `/dev/ttyACM0` на свой (`/dev/ttyUSB0`, `COM5` и т.д.).

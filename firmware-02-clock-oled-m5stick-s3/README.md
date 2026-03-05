# firmware-02-clock-oled-m5stick-s3

Новая версия прошивки часов для **M5Stack Stick S3 (ESP32-S3)**.

## Что внутри

- `main.ino` — скетч часов с синхронизацией времени через NTP и выводом времени/даты на экран.
- `merge_bin.sh` — собирает **один** файл `merged.bin` из `bootloader.bin + partitions.bin + firmware.bin`.
- `flash_esp32.sh` — прошивает ESP32-S3 **одним merged bin** (адрес `0x0`).

## Зависимости

- библиотека `M5Unified` (для сборки скетча)
- `esptool.py` (для merge и прошивки)

## Сборка

Собери скетч (Arduino IDE / arduino-cli / platformio), чтобы получить бинарники:

- `bootloader.bin`
- `partitions.bin`
- `firmware.bin`

## Сделать один bin (для прошивки с телефона)

```bash
./merge_bin.sh build/bootloader.bin build/partitions.bin build/firmware.bin build/m5stick_s3_clock_merged.bin
```

После этого у тебя будет один файл:

- `build/m5stick_s3_clock_merged.bin`

Именно его удобно передавать/прошивать с телефона через приложения, которые принимают один `.bin`.

## Прошивка merged.bin из терминала

```bash
./flash_esp32.sh /dev/ttyACM0 build/m5stick_s3_clock_merged.bin
```

Если порт другой — замени `/dev/ttyACM0` на свой (`/dev/ttyUSB0`, `COM5` и т.д.).

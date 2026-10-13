# Hardware reference

Board: **HackCard ESP32-S3** (ESP32-S3FH4R2) — see [`board_config.h`](../HackCard_ESP32/config/board_config.h) for authoritative pins.

| Function | GPIO / bus | Notes |
|----------|------------|--------|
| BOOT button | 0 | Short / long / double gestures |
| Buzzer | 37 | PWM |
| RGB ring (12× WS2812) | 45 | GRB order |
| Wi-Fi status LED (1× WS2812) | 38 | |
| PN532 NFC | I2C SDA 8, SCL 18, IRQ 17 | Address 0x24 |
| microSD | SPI CS 10, MOSI 11, CLK 12, MISO 13 | FAT32 |

Arduino library pin defines: [`HackCard_Pins.h`](../HackCard_ESP32/library/HackCard/src/HackCard_Pins.h).

## Power

Use a **data-capable USB-C** cable. Labs with Wi-Fi promiscuous mode + RGB draw more current — powered USB port or hub recommended.

## RF

Wi-Fi is **2.4 GHz only**. NFC is **reader/writer** (PN532) — not phone-style card emulation on ESP32.

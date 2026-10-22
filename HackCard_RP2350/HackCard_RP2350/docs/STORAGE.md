# Storage

| | No SD card | FAT32 microSD |
|--|------------|----------------|
| Contact card | `config/user_config.h` | Same header |
| Activity log | RAM, `log` command | Also `/logs/activity.log` |
| Badge example | Serial only | `/logs/badge_log.txt` |

Folders created when a card mounts:

- `/hackcard`
- `/nfc`
- `/payloads`
- `/logs`

There is no LittleFS config file in 0.1.0. The flash chip is 2MB and is used for the program.

SPI pins are fixed: CS 25, CLK 26, MOSI 27, MISO 28, on `SPI1`.

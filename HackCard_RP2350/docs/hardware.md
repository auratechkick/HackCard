# Hardware

Board: **HackCard RP2350**. Authoritative pins: [`config/board_config.h`](../HackCard_RP2350/config/board_config.h) and [`HackCard_Pins.h`](../library/HackCard_RP2350/src/HackCard_Pins.h).

Verified by the production-test sketch in `sketches/HackCard_Production_Test` (firmware 1.0.4).

| Function | GPIO | Bus |
|----------|------|-----|
| User button | 2 | Active low, internal pull-up |
| Buzzer | 9 | `tone()` / `noTone()` |
| PN532 SDA | 6 | **I2C1** (`Wire1`) |
| PN532 SCL | 7 | **I2C1** (`Wire1`) |
| PN532 IRQ | 11 | Present; UID reads poll instead of using the IRQ |
| RGB strip | 24 | 12× WS2812, GRB |
| SD CS | 25 | **SPI1** |
| SD CLK | 26 | **SPI1** |
| SD MOSI | 27 | **SPI1** |
| SD MISO | 28 | **SPI1** |

`Wire` and `SPI` (I2C0 / SPI0) do not reach these connectors. Use `Wire1` and `SPI1`.

## Not on this board

- Wi-Fi
- BLE
- A separate status LED (pixel 0 of the strip is the status pixel)
- BOOT and RUN as software GPIO (BOOT is QSPI, RUN is reset)

## Power and storage

USB-C, data cable. Flash is **2MB** (W25Q16). microSD is optional and must be FAT32. With no card, settings stay in `user_config.h` and logs stay in RAM.

## NFC

PN532 at I2C address `0x24`. Reader/writer, same role as the ESP32 board.

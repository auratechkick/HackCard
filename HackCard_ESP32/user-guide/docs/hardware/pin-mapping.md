# Pin mapping (ESP32-S3)

Authoritative source in firmware: [`HackCard_ESP32/config/board_config.h`](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/config/board_config.h) and library [`HackCard_Pins.h`](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/library/HackCard/src/HackCard_Pins.h).

Board: **HackCard ESP32-S3** · MCU **ESP32-S3FH4R2** · **4 MB** flash · **2 MB** OPI PSRAM.

---

## GPIO and buses

| Function | Pin / bus | Notes |
|----------|-----------|--------|
| **BOOT button** | GPIO **0** | Short / long / double; safe-mode hold |
| **Buzzer** | GPIO **37** | PWM |
| **Wi-Fi status LED** | GPIO **38** | 1× WS2812 |
| **RGB ring** | GPIO **45** | 12× WS2812, **GRB** order (`RGB_USE_GRB`) |
| **PN532 SDA** | GPIO **8** | I2C |
| **PN532 SCL** | GPIO **18** | I2C |
| **PN532 IRQ** | GPIO **17** | Optional interrupt |
| **PN532 address** | **0x24** | Fixed in firmware |
| **microSD CS** | GPIO **10** | SPI |
| **microSD MOSI** | GPIO **11** | SPI |
| **microSD CLK** | GPIO **12** | SPI |
| **microSD MISO** | GPIO **13** | SPI |
| **USB-C** | — | Power + USB CDC + USB OTG (HID) |
| **RESET** | — | Hardware reset (not a firmware GPIO) |

---

## What is not routed to GPIO

| Item | Notes |
|------|--------|
| **5 GHz Wi-Fi** | ESP32 is **2.4 GHz only** |
| **Phone NFC emulation** | PN532 is **reader/writer** only |

---

## Power

Use a **data-capable USB-C** cable. Wi-Fi lab + RGB can draw significant current — prefer a powered USB port or hub.

---

## Related docs

- [What's on the board](../getting-started/whats-included.md)
- In-repo reference: [hardware.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/docs/hardware.md)

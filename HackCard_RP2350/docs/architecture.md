# Firmware architecture (0.1.0)

```
HackCard_RP2350/
├── HackCard_RP2350.ino     setup/loop
├── config/
│   ├── board_config.h      pins, version
│   ├── user_config.h       contact card and brightness
│   └── app_registry.h      feature switches (Wi-Fi and BLE are 0)
└── src/SerialLab.cpp       line-oriented USB console
```

Drivers are the Arduino library `HackCard_RP2350` (`#include <HackCardRP2350.h>`). The firmware sketch does not carry a second copy.

## Boot

1. USB CDC serial at 115200
2. RGB strip, buzzer, user button
3. USB keyboard and mouse (Pico SDK HID, composite with CDC)
4. PN532 on I2C1
5. microSD on SPI1, then lab folders if the card mounts
6. `help` prompt

## What replaced the ESP32 web UI

| ESP32 | RP2350 0.1.0 |
|-------|----------------|
| Lab Console at `http://192.168.4.1` | Serial commands (`help`, `nfc`, `vcard`, `sd`, `hid`) |
| Wi-Fi apps | Not available — no radio |
| BLE contact card | NFC vCard write |
| LittleFS config JSON | `user_config.h` (recompile to change) |
| BOOT button gestures | User button on GPIO 2 |

## Still the same ideas

- One library, one example per feature
- Business-card identity in a config header
- Optional SD for logs (`/logs`, `/nfc`, `/payloads`, `/hackcard`)
- Ring patterns: idle, scan, success, error

## Not in 0.1.0

NFC permanent lock, MIFARE Classic tools, and a browser UI. The ported NFC driver still contains read, info, and NDEF write, which the serial lab and examples use.

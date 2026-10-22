# HackCard ESP32

Official firmware, Arduino library, and examples for the **HackCard ESP32-S3** security lab board.

| | |
|---|---|
| **Firmware** | `1.0.0` — Lab Console, NFC/Wi‑Fi/BLE/HID labs, Flash + SD storage |
| **Docs in repo** | `docs/` — Arduino setup, serial/logging, storage (full site TBD after testing) |
| **Board** | ESP32-S3FH4R2 · PN532 NFC · RGB ring · Wi-Fi · BLE · USB HID · microSD |

---

## What's in this repository

```
HackCard-ESP32/
├── HackCard_ESP32.ino     ← Full product firmware (web + CLI + all labs)
├── config/                ← Pins, user settings, feature flags
├── src/                   ← HAL, apps, core
├── partitions.csv
├── library/HackCard/      ← Arduino library + standalone examples
└── docs/                  ← Arduino setup, backer guide, Wi-Fi tips
```

| Path | Who it's for |
|------|----------------|
| **Full firmware** (`HackCard_ESP32.ino`) | Backers / lab users who want the dashboard + every feature |
| **Library + examples** (`library/HackCard/`) | Custom apps — [EXAMPLES.md](library/HackCard/EXAMPLES.md), [CUSTOM_DEVELOPMENT.md](docs/CUSTOM_DEVELOPMENT.md) |

---

## Quick start — full firmware

1. Install [Arduino IDE 2.x](https://www.arduino.cc/en/software) and ESP32 board support  
2. Install libraries: **Adafruit NeoPixel**, **ArduinoJson**, **Adafruit PN532**, **Adafruit BusIO**  
3. Edit `config/user_config.h` (name, AP password, lab PIN)  
4. Open `HackCard_ESP32.ino`  
5. Tools: **ESP32S3 Dev Module**, **USB CDC On Boot = Enabled**, **USB Mode = USB-OTG (TinyUSB)**, custom `partitions.csv`  
6. Upload → join AP `HackCard-Setup` / `hackcard2026` → open http://192.168.4.1  

**Lab Console:** Run `scripts/sync-web-ui.ps1` (from repo root, edits `../hackcard-web-ui/www`), then upload firmware. UI is **embedded** — works without SD. Optional **Sketch Data Upload** of `data/www/` to override UI on LittleFS.

Full steps: [Arduino Setup](docs/ARDUINO_SETUP.md) · [Backer Guide](docs/BACKER_GUIDE.md) · [Storage](docs/STORAGE.md) · [Serial & logging](docs/SERIAL_AND_LOGGING.md) · [Release notes](RELEASE.md)

---

## Quick start — library examples

1. Copy `library/HackCard` → `Documents/Arduino/libraries/HackCard`  
2. **File → Examples → HackCard**  
3. Start with `01_Basics → Hello_RGB_Ring` or `02_NFC → NFC_Read_UID`  

See [library/HackCard/README.md](library/HackCard/README.md).

### Example catalog

Every major **web dashboard** page has a matching Serial example. Full map: [`library/HackCard/EXAMPLES.md`](library/HackCard/EXAMPLES.md).

| Category | Examples |
|----------|----------|
| Start here | Web feature map, **Template_Custom_App** (fork this) |
| Basics | RGB ring, LED demos 1–12, buzzer, BOOT, Wi-Fi LED, RGB_And_Button |
| NFC | Read/info/dump, dump pages→SD, write URL/text/vCard, erase/clone/kill, Classic dump/clone/keyscan |
| Wi-Fi | AP, scan, STA connect, captive portal, beacon, evil-twin sim, training portal, probe sniff, deauth demo |
| BLE | Scan, advertise, GATT contact card |
| USB HID | Type, URL, apps/lock, media, mouse, OS menu, desk remote, Ducky Script, presentation/email/meeting |
| Storage / System | SD info, lab folders, diagnostics, profile/vCard, modes, activity log |
| Real life | Badge tap, guest portal, BLE card, desk light, NFC→HID lock, standup/meeting/guest Wi-Fi notes |

---

## Features (full firmware)

- **NFC / RFID** — PN532 reader (UID, NDEF, dump/clone labs). Reader only — no card emulation  
- **Wi-Fi lab** — AP, scan/connect, captive portal, beacon, training portal, monitor demos  
- **BLE** — scan, advertise, contact card  
- **USB HID** — keyboard / mouse / media payloads (TinyUSB)  
- **RGB + buzzer** — 12-LED ring + status LED + piezo feedback  
- **Storage** — LittleFS (Flash Mode) or microSD (Full Mode)  
- **Web dashboard** — status, NFC, Wi-Fi, BLE, HID, settings, diagnostics  

Disable apps at compile time in `config/app_registry.h`.

---

## Ethical use

Lab tools are for **education and authorized testing** on systems you own or have written permission to test. Change default PINs and AP password in `config/user_config.h` before sharing a device.

---

## Documentation (1.0.0 release)

| Topic | Link |
|-------|------|
| Release checklist | [RELEASE.md](RELEASE.md) |
| Arduino setup | [docs/ARDUINO_SETUP.md](docs/ARDUINO_SETUP.md) |
| Backer guide | [docs/BACKER_GUIDE.md](docs/BACKER_GUIDE.md) |
| Storage (flash vs SD) | [docs/STORAGE.md](docs/STORAGE.md) |
| Serial & logging | [docs/SERIAL_AND_LOGGING.md](docs/SERIAL_AND_LOGGING.md) |
| Changelog | [CHANGELOG.md](CHANGELOG.md) |

Full documentation hub: [`../docs/README.md`](../docs/README.md) (getting started, Lab Console, API).

---

## License

See [LICENSE](LICENSE).

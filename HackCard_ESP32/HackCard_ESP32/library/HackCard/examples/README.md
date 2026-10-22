# HackCard Arduino examples (release 1.0.0)

Standalone sketches for **custom development** — each maps to a feature in the full **Lab Console** firmware.

| Resource | Path |
|----------|------|
| **Full index (web → sketch)** | [`../EXAMPLES.md`](../EXAMPLES.md) |
| **How to install & fork** | [`../../docs/CUSTOM_DEVELOPMENT.md`](../../docs/CUSTOM_DEVELOPMENT.md) |
| **Cheat sheet on device** | Upload `00_Start_Here/Web_Feature_Map` |

## Folder layout

| Folder | Topics |
|--------|--------|
| `00_Start_Here` | Map + template for your own app |
| `01_Basics` | RGB ring, buzzer, BOOT button, Wi-Fi LED, RGB + button gestures |
| `02_NFC` | PN532 read/write/dump, page dump to SD, classic labs |
| `03_WiFi` | Scan, AP, portal, monitor, karma, capture |
| `04_BLE` | Scan, advertise, GATT contact card |
| `05_USB_HID` | Keyboard/mouse/media, OS menu, desk remote, Ducky Script, presentation & meeting templates |
| `05_Storage` | microSD mount and export folders |
| `06_RealLife` | Badge tap, desk light, NFC lock/unlock, standup/break notes, guest Wi-Fi card |
| `07_System` | Profile, modes, diagnostics, logs, factory reset |

## Every sketch header includes

- **FEATURE** — what you are learning  
- **FULL FIRMWARE** — matching Lab Console page  
- **HOW TO RUN** — board settings, Serial **115200**, steps  
- **EDIT ME** — where to change SSID, PIN, URL, etc.  
- **ETHICS** — on RF/tag/PC lab sketches  

Upload **one example at a time** (Wi-Fi and HID sketches change USB/Wi-Fi mode).

**Count:** 72 `.ino` files — full table in [`../EXAMPLES.md`](../EXAMPLES.md). RP2350-only flows live under `05_USB_HID` and `06_RealLife` (use `#include <HackCardUsb.h>`).

## Full product firmware

For the web dashboard and REST API, flash [`HackCard_ESP32.ino`](../../../HackCard_ESP32.ino) instead of these examples.

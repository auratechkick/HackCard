# HackCard

Hardware security lab platforms — **ESP32-S3** (Lab Console + Wi‑Fi/BLE) and **RP2350** (USB serial lab + NFC/HID).

| Folder | Board | Contents |
|--------|-------|----------|
| **[HackCard_ESP32/](HackCard_ESP32/)** | ESP32-S3 | Firmware 1.0.0, Lab Console web UI, **72** Arduino examples |
| **[HackCard_RP2350/](HackCard_RP2350/)** | RP2350 | Serial lab firmware 0.1.0, **40** Arduino examples, production test |

## Choose your board

| You have… | Start here |
|-------------|------------|
| **HackCard ESP32-S3** | [HackCard_ESP32/README.md](HackCard_ESP32/README.md) · [Docs](HackCard_ESP32/docs/README.md) |
| **HackCard RP2350** | [HackCard_RP2350/README.md](HackCard_RP2350/README.md) · [Docs](HackCard_RP2350/docs/README.md) |

The ESP32 tree includes Wi‑Fi, BLE, and the browser **Lab Console**. The RP2350 tree has **no radio** — control is USB serial, the user button, NFC, RGB strip, and USB HID to your PC.

Many **USB HID** and **NFC desk** examples were developed on RP2350 first; equivalent flows are also in the ESP32 library under `HackCard_ESP32/.../library/HackCard/examples/`.

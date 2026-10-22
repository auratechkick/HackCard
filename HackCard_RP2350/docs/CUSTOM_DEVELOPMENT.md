# Custom development (RP2350)

Use the **serial lab firmware** (`HackCard_RP2350/HackCard_RP2350.ino`) for an interactive USB console, or upload **one library example at a time** to learn a single feature.

## Example folders

```
library/HackCard_RP2350/examples/
├── 00_Start_Here/     Template_Custom_App
├── 01_Board/          RGB_And_Button
├── 02_NFC/            Read, dump pages, write, erase, kill
├── 03_USB_HID/        Keyboard, OS menu, desk remote, Ducky Script, …
├── 04_Storage/        SD info, lab folders
├── 05_RealLife/       NFC lock, standup notes, guest Wi-Fi card, …
└── 06_System/         Diagnostics
```

**Index:** [library/HackCard_RP2350/EXAMPLES.md](../library/HackCard_RP2350/EXAMPLES.md)

## Install the library

From this folder in PowerShell:

```powershell
cd HackCard_RP2350\scripts
.\install-library.ps1
```

Restart Arduino IDE → **File → Examples → HackCard RP2350**.

## Board settings (every sketch)

| Tools | Value |
|-------|--------|
| Board | **Generic RP2350** |
| Flash Size | **2MB (no FS)** |
| USB Stack | **Pico SDK** |

Do **not** select Adafruit TinyUSB for the core USB stack — it breaks the built-in **Keyboard** / **Mouse** libraries used by HID examples.

Serial Monitor: **115200**.

## HAL in `<HackCardRP2350.h>`

| Object | Hardware |
|--------|----------|
| `RgbRing` | 12× WS2812 (GPIO 24) |
| `Buzzer` | GPIO 9 |
| `Buttons` | User button GPIO 2 |
| `Nfc` | PN532 on **Wire1** (I2C1) |
| `UsbHid` | USB keyboard + mouse + media |
| `Host` | OS-aware PC shortcuts (Windows / macOS / Linux) |
| `Ducky` | Ducky Script runner |
| `HackSd` | microSD on **SPI1** (optional) |

Wi‑Fi and BLE symbols in ESP32 docs **do not apply** to this board.

## Serial lab vs examples

| | Serial lab firmware | Library example |
|--|---------------------|-----------------|
| Interface | USB commands (`help`, `nfc`, …) | One feature per upload |
| Best for | Daily driver on desk | Learning, demos, custom apps |
| Web UI | None | None |

Fork `00_Start_Here/Template_Custom_App` when you outgrow a single demo.

## ESP32 parity

If you also own **HackCard ESP32-S3**, the same HID/NFC *ideas* often exist as ESP32 examples with `#include <HackCardUsb.h>`. ESP32 adds Wi‑Fi/BLE/Lab Console — see [HackCard_ESP32 docs](../../HackCard_ESP32/docs/README.md).

## Ethics

Use NFC write/kill and USB HID only on **tags and PCs you own** or have **written permission** to test.

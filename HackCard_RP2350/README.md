# HackCard RP2350

Serial-lab firmware, Arduino library, and examples for the **HackCard RP2350** board.

This MCU has **no Wi-Fi and no BLE**. There is no Lab Console web UI — you use **USB serial** (115200), the **user button**, **NFC**, the **RGB strip**, and **USB HID** on the PC the board is plugged into.

| | |
|---|---|
| **Version** | 0.1.0 |
| **Core** | Earle Philhower `arduino-pico` (Generic RP2350) |
| **License** | [MIT](LICENSE) |
| **ESP32 sibling** | [../HackCard_ESP32/README.md](../HackCard_ESP32/README.md) |

---

## Who this is for

| Audience | Start here |
|----------|------------|
| **First flash** | [docs/getting-started.md](docs/getting-started.md) |
| **Pin map & limits** | [docs/hardware.md](docs/hardware.md) |
| **How firmware is organized** | [docs/architecture.md](docs/architecture.md) |
| **Write your own sketch** | [library/HackCard_RP2350/EXAMPLES.md](library/HackCard_RP2350/EXAMPLES.md) |
| **Factory / QC** | [sketches/HackCard_Production_Test/](sketches/HackCard_Production_Test/) |

Full documentation index: **[docs/README.md](docs/README.md)**

---

## Repository layout

```
HackCard_RP2350/
├── README.md                 ← you are here
├── docs/                     ← user + developer guides
├── HackCard_RP2350/          ← serial lab firmware (.ino + config + SerialLab)
├── library/HackCard_RP2350/  ← Arduino library + 40 examples
├── sketches/                 ← production test (hardware QC)
└── scripts/install-library.ps1
```

---

## Quick start (5 steps)

1. Arduino IDE 2.x → Boards Manager → **Raspberry Pi Pico/RP2040/RP2350** (Philhower).
2. Libraries: **Adafruit PN532**, **Adafruit BusIO**, **Adafruit NeoPixel**.
3. Run `scripts/install-library.ps1` (copies `library/HackCard_RP2350` into your Arduino libraries folder).
4. Open `HackCard_RP2350/HackCard_RP2350.ino` — board **Generic RP2350**, **2MB (no FS)**, **USB Stack: Pico SDK**.
5. Upload → Serial Monitor **115200** → type `help`.

Edit contact card fields in `HackCard_RP2350/config/user_config.h` before sharing a device.

---

## Features (0.1.0)

- **Serial lab** — `help`, NFC read/write, vCard, SD listing, HID demos over USB
- **NFC** (PN532 on I2C1) — UID, NDEF, Type 2 dump/erase/kill (authorized tags only)
- **USB HID** — keyboard/mouse/media to **your** PC (OS-aware shortcuts via library)
- **microSD** (SPI1, optional) — `/hackcard`, `/nfc`, `/payloads`, `/logs`, `/ducky`
- **RGB ring + buzzer + user button** (GPIO 2)

---

## Examples (no serial lab firmware)

**File → Examples → HackCard RP2350** in Arduino IDE (after `install-library.ps1`).

Master index: [library/HackCard_RP2350/EXAMPLES.md](library/HackCard_RP2350/EXAMPLES.md)

---

## Ethical use

Tools are for **education and authorized testing** on PCs, tags, and storage you own or have permission to use. See [docs/ethics-and-authorized-use.md](docs/ethics-and-authorized-use.md).

---

## Release notes

[HackCard_RP2350/RELEASE.md](HackCard_RP2350/RELEASE.md) · [CHANGELOG.md](HackCard_RP2350/CHANGELOG.md)

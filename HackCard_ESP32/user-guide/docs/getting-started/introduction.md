# Introduction

HackCard ESP32-S3 is a wallet-sized development board for **hands-on security education**: NFC tag work, controlled Wi-Fi labs, BLE demos, and USB HID exercises toward a host PC — all manageable from a built-in **Lab Console** web app.

---

## What you get in software

| Piece | What it does |
|-------|----------------|
| **HackCard_ESP32.ino** | Full firmware — soft AP, REST API, embedded Lab Console |
| **HackCard library** | **73** standalone Arduino examples (NFC, Wi-Fi, BLE, HID, system) |
| **hackcard-web-ui** | HTML/CSS/JS source synced into firmware flash |

You flash firmware over **USB-C** with Arduino IDE. Day-to-day control is usually **Wi-Fi + browser** at **`http://192.168.4.1/`**, with optional **Serial Monitor @ 115200** or the web **Terminal** page.

---

## What HackCard is

| Capability | Notes |
|------------|--------|
| **NFC read/write** | PN532 + PCB antenna — NTAG, Type 2 dump/clone labs, MIFARE Classic tools (authorized tags only) |
| **Wi-Fi lab** | Scan, AP, STA, captive portal, training portal, monitor, karma, capture — gated by Lab PIN |
| **BLE** | Scan, advertise, GATT contact card |
| **USB HID** | TinyUSB keyboard/mouse/media to a connected PC |
| **Feedback** | 12× WS2812 ring, Wi-Fi LED, piezo buzzer, BOOT button |
| **Storage** | Internal flash always; optional **microSD** for exports (Full Mode) |

---

## What HackCard is not

<div class="nfc-notice" markdown="1">

- **Not** an NFC **emulator** — the PN532 is a **reader/writer** for physical tags.
- **Not** a turnkey “hack any network” product — offensive-style tools are documented, PIN-gated, and for **authorized labs**.
- **Not** 5 GHz Wi-Fi — ESP32 is **2.4 GHz only**.
- **Not** dependent on SD for the dashboard — the Lab Console is **embedded in firmware**; SD adds export folders and capacity.

</div>

---

## Business mode vs Lab mode

After you join the HackCard Wi-Fi and open the Lab Console:

| Mode | Experience |
|------|----------------|
| **Business** | Public-style **digital profile** (contact, QR, vCard). Labs stay hidden until you unlock. |
| **Lab** | Full sidebar (desktop) or bottom tabs (phone): Wi-Fi, NFC, BLE, HID, Files, Settings, Terminal. |

Unlock uses **`LAB_MODE_PIN`** from `config/user_config.h` (default is a demo value — **change before sharing the device**).

Details: [Open the Lab Console](lab-console.md).

---

## Who this guide is for

| You are… | Start here |
|----------|------------|
| **New owner — first flash** | [What's on the board](whats-included.md) → [First setup](first-setup.md) |
| **Backer / demo user** | [Personalize](personalize.md) → [Lab Console](lab-console.md) |
| **Developer** | After this guide: monorepo `HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md` and library [EXAMPLES.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/library/HackCard/EXAMPLES.md) |

---

## Firmware version

Current release in this tree: **`1.0.2`** (`FIRMWARE_VERSION` in `HackCard_ESP32/config/board_config.h`).

Always treat **`board_config.h`** and **`user_config.h`** as authoritative for your checkout.

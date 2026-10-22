# HackCard ESP32

**ESP32-S3 security lab board** — full firmware, **Lab Console** web UI, and **72 Arduino examples** for custom development.

| | |
|---|---|
| **Version** | 1.0.0 |
| **License** | [MIT](LICENSE) |
| **Documentation** | **[Start here → `docs/README.md`](docs/README.md)** |

---

## Who this repo is for

| Audience | Start here |
|----------|------------|
| **I just got the board** | [Getting started](docs/getting-started.md) |
| **I only use the browser UI** | [Lab Console user guide](docs/lab-console-user-guide.md) |
| **I flash firmware / develop** | [Arduino setup](HackCard_ESP32/docs/ARDUINO_SETUP.md) · [Architecture](docs/architecture.md) |
| **I write my own sketches** | [Examples index](HackCard_ESP32/library/HackCard/EXAMPLES.md) · [Custom development](HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md) |
| **I automate the device** | [API overview](docs/api-overview.md) |
| **HackCard RP2350 (USB serial lab)** | [HackCard_RP2350/docs/README.md](../HackCard_RP2350/docs/README.md) |

---

## Features

- **Lab Console** — responsive web app at `http://192.168.4.1/` (embedded in firmware)
- **Business / Lab modes** — profile portal + PIN-gated security labs
- **NFC** (PN532) — read, write, dump, Classic lab tools
- **Wi-Fi** — scan, AP, portal, training, monitor, karma, EAPOL capture
- **BLE** — scan, advertise, GATT contact card
- **USB HID** — keyboard/mouse/media, OS shortcuts, Ducky Script runner (TinyUSB)
- **Storage** — Flash Mode without SD; Full Mode exports with microSD
- **HackCard Arduino library** — 72 standalone sketches (Lab Console + RP2350-parity HID/NFC demos)

---

## Quick start (5 steps)

1. Install Arduino IDE 2.x + ESP32-S3 board support ([details](HackCard_ESP32/docs/ARDUINO_SETUP.md)).
2. Edit [`HackCard_ESP32/config/user_config.h`](HackCard_ESP32/config/user_config.h) (name, AP password, **Lab PIN**).
3. Open [`HackCard_ESP32/HackCard_ESP32.ino`](HackCard_ESP32/HackCard_ESP32.ino) and upload.
4. Join the HackCard Wi-Fi (2.4 GHz) — credentials on Serial **@ 115200**.
5. Browser → **`http://192.168.4.1/`**

---

## Repository layout

```
├── README.md                 ← you are here
├── docs/                     ← documentation hub (users + developers)
├── LICENSE
├── HackCard_ESP32/           ← firmware sketch + src + config
│   ├── library/HackCard/     ← Arduino library + examples/
│   └── docs/                 ← setup, storage, serial, backer guide
└── hackcard-web-ui/www/      ← Lab Console source (sync → firmware)
```

---

## Web UI developers

Edit `hackcard-web-ui/www/`, then:

```powershell
cd HackCard_ESP32\scripts
.\sync-web-ui.ps1
```

Rebuild and upload firmware. See [hackcard-web-ui/README.md](hackcard-web-ui/README.md).

---

## Examples (no web UI)

**File → Examples → HackCard** in Arduino IDE.  
Every sketch includes a **release 1.0.0** header (feature, board settings, how to run).

Full table: [`HackCard_ESP32/library/HackCard/EXAMPLES.md`](HackCard_ESP32/library/HackCard/EXAMPLES.md)

---

## Ethics

Wi-Fi, NFC, and HID tools are for **authorized education and testing**.  
Read [ethics-and-authorized-use.md](docs/ethics-and-authorized-use.md) and change default PINs before sharing hardware.

---

## Release

- Checklist: [`HackCard_ESP32/RELEASE.md`](HackCard_ESP32/RELEASE.md)  
- Changelog: [`HackCard_ESP32/CHANGELOG.md`](HackCard_ESP32/CHANGELOG.md)

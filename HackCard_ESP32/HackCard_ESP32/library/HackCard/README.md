# HackCard Arduino Library

Hardware drivers and **70+ Serial examples** covering every major Lab Console feature — for custom development without the full web firmware.

Full product firmware (dashboard + REST API): [`HackCard_ESP32.ino`](../../HackCard_ESP32.ino).

| | |
|---|---|
| **Version** | 1.0.0 |
| **Example index** | [EXAMPLES.md](EXAMPLES.md) |
| **Custom dev guide** | [docs/CUSTOM_DEVELOPMENT.md](../../docs/CUSTOM_DEVELOPMENT.md) |

---

## Install

1. Copy this folder to `Documents/Arduino/libraries/HackCard` **or** open examples from this repo.
2. Install **Adafruit NeoPixel**, **Adafruit PN532**, **Adafruit BusIO**.
3. Board: **ESP32S3 Dev Module** · USB CDC On Boot **Enabled** · **USB-OTG (TinyUSB)** for HID sketches.

---

## Quick start

1. **File → Examples → HackCard → 00_Start_Here → Web_Feature_Map** — prints the full map on Serial @ 115200.
2. Pick a feature folder (`01_Basics` … `07_System`) from [EXAMPLES.md](EXAMPLES.md).
3. Edit **EDIT ME** constants, upload one sketch at a time.

**Build your own:** fork `00_Start_Here/Template_Custom_App`.

---

## Example groups

| Folder | Covers |
|--------|--------|
| `01_Basics` | Ring control, demos, buzzer, BOOT, Wi-Fi LED |
| `02_NFC` | Read, dump, write, erase, clone, kill, Mifare Classic |
| `03_WiFi` | Scan, AP, portal, beacon, twin, training, monitor, karma, EAPOL |
| `04_BLE` | Scan, advertise, contact card |
| `05_USB_HID` | Keyboard, mouse, media, URL, lock |
| `05_Storage` | SD info, lab export folders |
| `06_RealLife` | End-to-end demos |
| `07_System` | Profile, modes, diagnostics, activity log |

---

## API snapshot

```cpp
#include <HackCard.h>

void setup() {
  Serial.begin(115200);
  RgbRing.begin(60);
  WifiLed.begin(70);
  Buzzer.begin(true);
  Buttons.begin();
  Nfc.begin();
}
```

Pin map: `HackCard_Pins.h` (matches `config/board_config.h` in full firmware).

---

## Ethical use

Lab features are for **education and authorized testing** on systems you own or have written permission to test. Change default lab PINs before sharing hardware.

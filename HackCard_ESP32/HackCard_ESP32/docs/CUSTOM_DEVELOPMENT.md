# Custom development (without the full web firmware)

Many backers run the **full firmware** (`HackCard_ESP32.ino`) for the Lab Console. You can also build **your own sketches** using the Arduino library and Serial examples.

## Where the examples live

```
HackCard_ESP32/library/HackCard/examples/
├── 00_Start_Here/     Web_Feature_Map, Template_Custom_App
├── 01_Basics/         Ring, buzzer, BOOT button, Wi-Fi status LED, RGB_And_Button
├── 02_NFC/            Read, dump, dump pages→SD, write, classic labs
├── 03_WiFi/           Scan, AP, portal, monitor, karma, capture, …
├── 04_BLE/            Scan, advertise, GATT contact card
├── 05_USB_HID/        Keyboard / mouse / media + OS shortcuts, desk remote, Ducky Script
├── 05_Storage/        SD mount, lab folder layout
├── 06_RealLife/       Product demos + NFC→HID lock, standup/meeting notes, guest Wi-Fi card
└── 07_System/         Profile, modes, diagnostics, activity log
```

**Master index:** [`library/HackCard/EXAMPLES.md`](../library/HackCard/EXAMPLES.md) — **72 sketches** (Lab Console map + HackCard RP2350-parity HID/NFC flows).

Each example `.ino` begins with a **release 1.0.0** comment block (feature, full-firmware page, board settings, how to run, libraries, ethics). See also [`library/HackCard/examples/README.md`](../library/HackCard/examples/README.md).

## Install the library

1. Copy `library/HackCard` to `Documents/Arduino/libraries/HackCard`  
   **or** open examples directly from this repo folder.
2. Board: **ESP32S3 Dev Module**, **USB CDC On Boot = Enabled**.
3. For HID examples: **USB Mode = USB-OTG (TinyUSB)**.
4. Libraries: Adafruit NeoPixel, PN532, BusIO (see `library.properties`).

## Workflow

| Step | Action |
|------|--------|
| 1 | Pick an example closest to your goal (see EXAMPLES.md). |
| 2 | Edit **EDIT ME** constants (SSID, PIN, URL, name). |
| 3 | Upload **one sketch at a time** (Wi-Fi mode changes conflict with full firmware). |
| 4 | Serial Monitor **115200** — same baud as [`SERIAL_AND_LOGGING.md`](SERIAL_AND_LOGGING.md). |
| 5 | Fork `Template_Custom_App` when you outgrow a single-feature demo. |

## HAL included in `<HackCard.h>`

| Object | Hardware |
|--------|----------|
| `RgbRing` | 12× WS2812 ring (GPIO 45) |
| `WifiLed` | 1× WS2812 at Wi-Fi icon (GPIO 38) |
| `Buzzer` | PWM buzzer (GPIO 37) |
| `Buttons` | BOOT button gestures |
| `Nfc` | PN532 over I2C |
| `Ble` | BLE scan / advertise helpers |

Wi-Fi lab examples use **`WiFi.h`** / **`esp_wifi.h`** directly.

### USB HID helpers (`HackCardUsb.h`)

Sketches ported from **HackCard RP2350** (and advanced HID demos) include:

```cpp
#include <HackCard.h>
#include <HackCardUsb.h>
```

| Object | Role |
|--------|------|
| `UsbHid` | Keyboard, consumer (media), mouse |
| `Host` | OS-aware shortcuts (lock, screenshot, open URL, notes, clipboard) |
| `Ducky` | Run Ducky Script text (see `HID_Ducky_Scripts`) |
| `HackSd` | Optional microSD for `/nfc`, `/ducky`, exports |

Board: **USB Mode = USB-OTG (TinyUSB)**. Simpler HID tutorials may use `USBHIDKeyboard` directly — see `HID_Type_Hello`.

## Full firmware vs examples

| | Full firmware | Library example |
|--|---------------|-----------------|
| Web UI | Yes (embedded Lab Console) | No |
| REST API | Yes | No |
| Storage manager | Flash + SD tiers | Per-sketch (often Serial only) |
| Best for | Product demo, all labs | Learning, custom apps, automation |

To reuse logic from the big firmware (e.g. exact PCAP export), read `src/apps/` in this repo and copy the pieces you need into your sketch — examples stay small on purpose.

## Ethics

Change default lab PINs. Use Wi-Fi / NFC / HID tools only on **hardware and networks you own** or have **written permission** to test.

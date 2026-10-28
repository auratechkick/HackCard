# Write your own sketch

You can use HackCard in two ways:

1. **Full firmware** — `HackCard_ESP32.ino` + Lab Console (best for all-in-one demos).
2. **Custom Arduino sketch** — `#include <HackCard.h>` and only the drivers you need (best for learning and single-purpose tools).

---

## Start from the template

Open **`00_Start_Here/Template_Custom_App`** in the HackCard examples.

Typical pattern:

```cpp
#include <HackCard.h>

void setup() {
  Serial.begin(115200);
  RgbRing.begin(80);
  WifiLed.begin(70);
  Buzzer.begin(true);
  Buttons.begin();
  // Add only what you need: Nfc.begin(), etc.
}

void loop() {
  // Your logic
}
```

Remove unused peripherals to keep uploads small and behavior clear.

---

## Pin map and board config

| Reference | File |
|-----------|------|
| Arduino library pins | `library/HackCard/src/HackCard_Pins.h` |
| Full firmware pins | `HackCard_ESP32/config/board_config.h` |
| Your name, AP, PINs | `HackCard_ESP32/config/user_config.h` (full firmware only) |

Examples use the **library** pin map — it matches the production board when you select **ESP32S3 Dev Module** with the project partition table.

---

## USB HID and SD helpers

After `#include <HackCard.h>`:

```cpp
#include <HackCardUsb.h>  // UsbHid, Host, Ducky, HackSd — HID / Ducky / SD-heavy sketches
```

Requires **USB-OTG (TinyUSB)** in Arduino **Tools**.

---

## When to stay on full firmware instead

| Need | Use |
|------|-----|
| Browser Lab Console, REST API, web Terminal | **`HackCard_ESP32.ino`** |
| One feature, Serial-only, classroom step-by-step | **Library example** or **Template_Custom_App** |
| Change web UI layout | Edit `hackcard-web-ui/www/`, sync into `data/www/`, reflash firmware |

Developer guide in the repo:  
[CUSTOM_DEVELOPMENT.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md)

---

## Suggested learning path

1. Flash full firmware and explore the Lab Console ([Lab Console](lab-console.md)).
2. Run **`Web_Feature_Map`** on Serial to see example names.
3. Pick one folder (`01_Basics` … `07_System`) and upload one sketch per day.
4. Fork **`Template_Custom_App`** into your own `.ino` for a custom lab tool.

---

## Defaults reminder

First login: [Default Wi-Fi, PINs, and URLs](default-credentials.md).

# Write your own sketch

Two paths on RP2350:

1. **Serial lab firmware** — `HackCard_RP2350.ino` + terminal commands (`help`, `nfc`, …).
2. **Custom example** — `#include <HackCardRP2350.h>` (or library header used in examples) and upload a single-purpose sketch.

---

## Start from the template

**`00_Start_Here/Template_Custom_App`**

Typical setup:

```cpp
#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  // RgbStrip.begin(...), Buzzer, Buttons, Nfc.begin(), etc.
}

void loop() {
  // Your logic
}
```

Include only the drivers you need.

---

## Pin map

| Reference | File |
|-----------|------|
| Library pins | `library/HackCard_RP2350/src/HackCard_Pins.h` |
| Firmware | `HackCard_RP2350/config/board_config.h` |
| Your profile / PIN | `HackCard_RP2350/config/user_config.h` |

---

## USB HID notes

- **USB Stack: Pico SDK** in Arduino **Tools** (required for Keyboard/Mouse in core).
- HID output goes to the **USB host** — the PC connected by USB-C.
- For Ducky-style payloads, see **`HID_Ducky_Scripts`** and `/ducky` on SD.

Developer guide: [CUSTOM_DEVELOPMENT.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_RP2350/docs/CUSTOM_DEVELOPMENT.md)

---

## When to use ESP32 instead

| Need | Board |
|------|--------|
| Browser Lab Console, Wi-Fi, BLE | **ESP32-S3** — [ESP32 guide](https://auratechkick.github.io/HackCard/esp32/) |
| USB-only classroom, smaller core | **RP2350** (this guide) |

---

## Learning path

1. Flash **`HackCard_RP2350.ino`**, run **`help`** ([Serial lab](serial-lab.md)).
2. Upload one sketch per folder from **`01_Board`** through **`03_USB_HID`**.
3. Fork **`Template_Custom_App`** for your own tool.

Defaults: [Default PINs & serial](default-credentials.md).

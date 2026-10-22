# HackCard RP2350 Arduino library

Hardware drivers and **41 Serial examples** for NFC, USB HID, RGB, buzzer, button, and optional microSD — **no Wi‑Fi or BLE**.

Product firmware (serial lab): [`../../HackCard_RP2350/HackCard_RP2350.ino`](../../HackCard_RP2350/HackCard_RP2350.ino).

| | |
|---|---|
| **Version** | 0.1.0 |
| **Example index** | [EXAMPLES.md](EXAMPLES.md) |
| **Custom dev guide** | [../../docs/CUSTOM_DEVELOPMENT.md](../../docs/CUSTOM_DEVELOPMENT.md) |

## Install

```powershell
cd HackCard_RP2350\scripts
.\install-library.ps1
```

Or copy this folder to `Documents/Arduino/libraries/HackCard_RP2350`.

Libraries: **Adafruit PN532**, **Adafruit BusIO**, **Adafruit NeoPixel**.  
Board: **Generic RP2350**, **2MB (no FS)**, **USB Stack: Pico SDK**.

## Quick start

1. **File → Examples → HackCard RP2350 → 00_Start_Here → Template_Custom_App**
2. Serial Monitor **115200**
3. Pick features from [EXAMPLES.md](EXAMPLES.md)

## API snapshot

```cpp
#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  RgbRing.begin(60);
  Buzzer.begin(true);
  Buttons.begin();
  Nfc.begin();
  UsbHid.begin();  // HID examples
}
```

Pin map: `HackCard_Pins.h` · matches [`../../HackCard_RP2350/config/board_config.h`](../../HackCard_RP2350/config/board_config.h).

## ESP32 sibling

HackCard ESP32-S3 adds Lab Console + Wi‑Fi/BLE; many HID/NFC demos were ported to [HackCard_ESP32](../../../HackCard_ESP32/HackCard_ESP32/library/HackCard/EXAMPLES.md).

# Custom sketches

1. Install the library (`scripts/install-library.ps1`).
2. **File → Examples → HackCard RP2350 → 00_Start_Here → Template_Custom_App**.
3. Save a copy and delete the parts you do not need.

```cpp
#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  RgbRing.begin(50);
  Buzzer.begin(true);
  Buttons.begin();
  Nfc.begin();      // I2C1, GPIO 6/7
  HackSd.begin();   // SPI1, GPIO 25-28
}

void loop() {
  RgbRing.update();
  Buttons.update();
}
```

Call `RgbRing.update()` and `Buttons.update()` from `loop()`. NFC and SD are polled when you call them.

Do not include `<HackCard.h>` in an RP2350 sketch. That header belongs to the ESP32 library.

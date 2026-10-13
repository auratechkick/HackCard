/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: BLE advertise
 * FULL FIRMWARE: /ble Advertise
 *
 * WHAT IT DOES:
 * Advertises a custom BLE name from the HackCard.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Use nRF Connect on phone to see the device.
 *
 * LIBRARIES: HackCard, BLE
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

static const char* kName = "HackCard-Demo";

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — BLE Advertise");

  RgbRing.begin(40);
  if (!Ble.startAdvertising(kName)) {
    Serial.println("ERROR: BLE advertise failed");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  Serial.printf("Advertising as \"%s\" — open a BLE scanner app.\n", kName);
  RgbRing.setPattern(RingPattern::Success);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(20);
}

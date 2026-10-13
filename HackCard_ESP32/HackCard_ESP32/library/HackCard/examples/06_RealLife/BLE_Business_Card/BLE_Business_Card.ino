/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: BLE business card
 * FULL FIRMWARE: /ble Contact
 *
 * WHAT IT DOES:
 * Advertises contact info for conference networking.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Scan with phone; edit name/URL in EDIT ME.
 *
 * LIBRARIES: HackCard, BLE
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

static const char* kDisplayName = "HackCard — Alex Rivera";

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — BLE Business Card");
  Serial.printf("Advertising: %s\n", kDisplayName);

  RgbRing.begin(50);
  Buzzer.begin(true);
  WifiLed.begin(40);

  if (!Ble.startAdvertising(kDisplayName)) {
    Serial.println("BLE advertise failed");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  WifiLed.setState(WifiLedState::ApActive);
  Serial.println("Phone: open a BLE scanner and look for the name above.");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();
  delay(20);
}

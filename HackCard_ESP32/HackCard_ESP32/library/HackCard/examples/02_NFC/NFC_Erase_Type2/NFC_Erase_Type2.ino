/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC erase Type 2
 * FULL FIRMWARE: /nfc Erase
 *
 * WHAT IT DOES:
 * Clears user data on a writable Type 2 tag (lab).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Confirm PIN if required; tag must be yours.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kLabPin = "1234";  // EDIT ME

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Erase Type 2 (web: /nfc erase)");
  Serial.println("Authorized lab use only. Data wipe is permanent for user pages.");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.printf("Type lab PIN (expect %s) then Enter...\n", kLabPin);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static bool armed = false;
  if (!armed) {
    if (!Serial.available()) return;
    String pin = Serial.readStringUntil('\n');
    pin.trim();
    if (pin != kLabPin) {
      Serial.println("PIN rejected.");
      Buzzer.playError();
      return;
    }
    armed = true;
    Serial.println("PIN OK. Place Type 2 tag to erase...");
    RgbRing.setPattern(RingPattern::Scanning);
    return;
  }

  RgbRing.update();
  char err[64] = {0};
  if (!Nfc.eraseType2UserData(err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Fail: %s\n", err);
      delay(800);
    }
    return;
  }
  Serial.println("OK — user data erased.");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  armed = false;
  delay(2000);
  Serial.println("Type PIN again to erase another tag...");
  RgbRing.setPattern(RingPattern::Idle);
}

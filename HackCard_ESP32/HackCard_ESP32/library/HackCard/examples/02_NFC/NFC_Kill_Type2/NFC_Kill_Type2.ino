/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC kill Type 2
 * FULL FIRMWARE: /nfc Kill
 *
 * WHAT IT DOES:
 * Permanently locks a Type 2 tag (irreversible lab demo).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Use sacrificial tags only.
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
  Serial.println("HackCard — NFC Kill Type 2 (web: /nfc kill)");
  Serial.println("WARNING: permanent lock. Authorized lab tags only.");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.printf("Type lab PIN (%s) then Enter to arm...\n", kLabPin);
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
    Serial.println("ARMED. Place Type 2 tag to lock permanently...");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  RgbRing.update();
  char err[64] = {0};
  if (!Nfc.killType2Tag(err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Fail: %s\n", err);
      delay(800);
    }
    return;
  }
  Serial.println("Tag locked.");
  Buzzer.playNfcKillOk();
  RgbRing.setPattern(RingPattern::Success);
  armed = false;
  delay(2000);
  Serial.println("Type PIN again to arm another kill...");
  RgbRing.setPattern(RingPattern::Idle);
}

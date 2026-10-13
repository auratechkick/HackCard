/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC write text
 * FULL FIRMWARE: /nfc Write text
 *
 * WHAT IT DOES:
 * Writes plain NDEF text to Type 2.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit message string; use a tag you own.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kText = "Hello from HackCard";

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Write Text (web: /nfc write text)");
  Serial.printf("Payload: \"%s\"\n", kText);

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Place a writable Type 2 tag...");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  char err[64] = {0};
  if (!Nfc.writeNdefText(kText, err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Fail: %s\n", err);
      Buzzer.playError();
      RgbRing.setPattern(RingPattern::Error);
      delay(1200);
      RgbRing.setPattern(RingPattern::Scanning);
    }
    return;
  }
  Serial.println("OK — text written. Read with a phone NFC app.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  delay(2500);
  RgbRing.setPattern(RingPattern::Scanning);
}

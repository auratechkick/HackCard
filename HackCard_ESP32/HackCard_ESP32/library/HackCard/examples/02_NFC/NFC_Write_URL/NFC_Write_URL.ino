/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC write URL
 * FULL FIRMWARE: /nfc Write URL
 *
 * WHAT IT DOES:
 * Writes an NDEF URL record to a blank Type 2 tag.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit URL constant; place tag; confirm write on Serial.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

// Change this URL before uploading
static const char* kUrl = "https://hardwarehackspace.github.io/hackcard-docs/";

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — NFC Write URL");
  Serial.printf("Target URL: %s\n", kUrl);

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }

  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.println("Place a writable Type 2 tag on the coil...");
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  char err[64] = {0};
  if (!Nfc.writeNdefUri(kUrl, err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Write failed: %s\n", err);
      RgbRing.setPattern(RingPattern::Error);
      Buzzer.playError();
      delay(1500);
      RgbRing.setPattern(RingPattern::Scanning);
    }
    return;
  }

  Serial.println("URL written successfully. Scan with a phone NFC app.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  delay(3000);
  RgbRing.setPattern(RingPattern::Scanning);
}

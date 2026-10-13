/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC tag info
 * FULL FIRMWARE: /nfc Info
 *
 * WHAT IT DOES:
 * Shows tag type and NDEF preview text when available.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Tap tags; compare output with dashboard Info tab.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — NFC Tag Info");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }

  Serial.println("Place a tag on the coil...");
  RgbRing.begin(40);
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  NfcTagDetail detail;
  if (!Nfc.readTagDetail(detail, 1200) || !detail.detected) {
    return;
  }

  Serial.println("--------");
  Serial.printf("UID:     %s\n", detail.uidHex);
  Serial.printf("Type:    %s\n", detail.tagType);
  Serial.printf("Family:  %s\n", detail.family);
  Serial.printf("Memory:  %s\n", detail.memoryHint);
  Serial.printf("NDEF:    %s\n", detail.ndefPreview[0] ? detail.ndefPreview : "(none)");

  RgbRing.setPattern(RingPattern::Success);
  delay(1500);
  RgbRing.setPattern(RingPattern::Scanning);
}

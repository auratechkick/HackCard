/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC read UID
 * FULL FIRMWARE: /nfc Read
 *
 * WHAT IT DOES:
 * Polls PN532 and prints tag UID when a tag is present.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Hold a tag on the coil; UID prints once per tap.
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
  Serial.println("HackCard — NFC Read UID");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected on I2C (SDA=8, SCL=18)");
    while (true) delay(1000);
  }

  Serial.printf("Chip: %s\n", Nfc.getChipInfo());
  Serial.println("Place a tag on the coil...");
  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 800)) {
    return;
  }

  if (!tag.detected) {
    return;
  }

  Serial.printf("UID: %s  (%u bytes)  type=%s\n",
                tag.uidHex, tag.uidLength, tag.tagType);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(1200);
  RgbRing.setPattern(RingPattern::Scanning);
}

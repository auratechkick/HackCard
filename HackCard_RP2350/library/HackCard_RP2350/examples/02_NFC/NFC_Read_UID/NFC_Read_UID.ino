/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: NFC read UID
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * PN532 is on I2C1 (SDA GPIO6, SCL GPIO7).
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Read UID");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected on I2C1 (SDA=6, SCL=7)");
    while (true) delay(1000);
  }

  Serial.printf("Chip: %s\n", Nfc.getChipInfo());
  Serial.println("Place a tag on the coil...");
  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
}

void loop() {
  RgbRing.update();

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 800) || !tag.detected) {
    return;
  }

  Serial.printf("UID: %s  (%u bytes)  type=%s\n", tag.uidHex, tag.uidLength, tag.tagType);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(1200);
  RgbRing.setPattern(RingPattern::Scanning);
}

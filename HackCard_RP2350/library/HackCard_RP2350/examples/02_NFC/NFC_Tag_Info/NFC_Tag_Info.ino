/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: NFC tag info
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Tag Info");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }

  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Hold a tag on the coil...");
}

void loop() {
  RgbRing.update();

  NfcTagDetail tag;
  if (!Nfc.readTagDetail(tag, 1000) || !tag.detected) {
    return;
  }

  Serial.printf("UID:    %s\n", tag.uidHex);
  Serial.printf("Type:   %s\n", tag.tagType);
  Serial.printf("Family: %s\n", tag.family);
  if (tag.ndefPreview[0]) {
    Serial.printf("NDEF:   %s\n", tag.ndefPreview);
  }
  if (tag.memoryHint[0]) {
    Serial.printf("Memory: %s\n", tag.memoryHint);
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(1500);
  RgbRing.setPattern(RingPattern::Scanning);
}

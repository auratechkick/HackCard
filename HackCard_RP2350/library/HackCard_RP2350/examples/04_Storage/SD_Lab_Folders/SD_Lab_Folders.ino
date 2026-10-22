/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: SD lab folders
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * Creates /hackcard /nfc /payloads /logs on a FAT32 card.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — SD Lab Folders");

  RgbRing.begin(40);
  Buzzer.begin(true);

  if (!HackSd.begin()) {
    Serial.println("ERROR: SD mount failed.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  if (!HackSd.ensureLabFolders()) {
    Serial.println("ERROR: Could not create folders.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  HackSd.printRootListing(Serial);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::FullMode);
  Serial.println("Folders ready.");
}

void loop() {
  RgbRing.update();
}

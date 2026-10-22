/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Conference badge tap
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * Logs UID lines to /logs/badge_log.txt when a FAT32 card is inserted.
 * ETHICS: Read only badges you are allowed to check in.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Conference Badge Tap");

  RgbRing.begin(55);
  Buzzer.begin(true);

  if (HackSd.begin()) {
    HackSd.ensureLabFolders();
    Serial.println("SD logging: ON");
    RgbRing.setPattern(RingPattern::FullMode);
  } else {
    Serial.println("SD logging: OFF (optional)");
    RgbRing.setPattern(RingPattern::FlashMode);
  }

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }

  Serial.println("Ready — tap badges on the coil.");
  delay(400);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  RgbRing.update();

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 600) || !tag.detected) {
    return;
  }

  const uint32_t ms = millis();
  Serial.printf("[%lu] CHECK-IN  UID=%s  type=%s\n", ms, tag.uidHex, tag.tagType);

  if (HackSd.isMounted()) {
    char line[96];
    snprintf(line, sizeof(line), "%lu,%s,%s", ms, tag.uidHex, tag.tagType);
    HackSd.appendLine("/logs/badge_log.txt", line);
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(700);
  RgbRing.setPattern(RingPattern::Idle);
}

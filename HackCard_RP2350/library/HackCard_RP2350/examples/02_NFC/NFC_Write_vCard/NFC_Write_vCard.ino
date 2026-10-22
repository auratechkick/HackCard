/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: NFC write vCard
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Write only tags you own. Edit the contact fields before upload.
 */

#include <HackCardRP2350.h>

static const char* kVcard =
  "BEGIN:VCARD\r\n"
  "VERSION:3.0\r\n"
  "FN:Your Name\r\n"
  "ORG:Your Company\r\n"
  "TITLE:Security Researcher\r\n"
  "EMAIL:you@example.com\r\n"
  "URL:https://example.com\r\n"
  "END:VCARD\r\n";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Write vCard");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }

  RgbRing.begin(50);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Place a writable Type 2 tag on the coil...");
}

void loop() {
  RgbRing.update();

  char err[64] = {0};
  if (!Nfc.writeNdefVcard(kVcard, err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Write failed: %s\n", err);
      RgbRing.setPattern(RingPattern::Error);
      Buzzer.playError();
      delay(1500);
      RgbRing.setPattern(RingPattern::Scanning);
    }
    return;
  }

  Serial.println("vCard written. A phone should offer to save the contact.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  delay(3000);
  RgbRing.setPattern(RingPattern::Scanning);
}

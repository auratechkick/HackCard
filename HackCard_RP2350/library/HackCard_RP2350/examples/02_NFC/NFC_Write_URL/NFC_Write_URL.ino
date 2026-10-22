/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: NFC write URL
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Write only tags you own.
 */

#include <HackCardRP2350.h>

static const char* kUrl = "https://example.com";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Write URL");
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

  Serial.println("URL written. Scan it with a phone NFC app.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  delay(3000);
  RgbRing.setPattern(RingPattern::Scanning);
}

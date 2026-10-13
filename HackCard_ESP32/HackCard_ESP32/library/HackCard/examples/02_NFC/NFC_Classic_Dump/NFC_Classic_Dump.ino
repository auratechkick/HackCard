/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Mifare Classic dump
 * FULL FIRMWARE: /nfc Classic dump
 *
 * WHAT IT DOES:
 * Attempts Classic sector read/key flow; prints dump to Serial.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Place Classic tag; authorized tags only.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kKeyHex = "FFFFFFFFFFFF";  // EDIT ME

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Classic Dump (web: /nfc classic dump)");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.printf("Key: %s\nPlace Classic 1K tag...\n", kKeyHex);
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  uint8_t key[6];
  char err[64] = {0};
  if (!Nfc.parseClassicKeyHex(kKeyHex, key, err, sizeof(err))) {
    Serial.printf("Bad key: %s\n", err);
    delay(2000);
    return;
  }

  NfcClassicDump dump;
  if (!Nfc.dumpClassicTag(key, dump, 16, 2000) || !dump.success) {
    if (dump.error[0]) {
      Serial.printf("Fail: %s\n", dump.error);
      delay(800);
    }
    return;
  }

  Serial.printf("UID=%s sectors=%u\n", dump.tag.uidHex, dump.sectorCount);
  for (uint8_t s = 0; s < dump.sectorCount; s++) {
    Serial.printf("Sector %u readable=%s\n", s, dump.sectors[s].readable ? "yes" : "no");
  }
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(2500);
  RgbRing.setPattern(RingPattern::Scanning);
}

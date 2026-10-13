/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Mifare Classic keyscan
 * FULL FIRMWARE: /nfc Classic keyscan
 *
 * WHAT IT DOES:
 * Tries common keys against Classic sectors (training).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Own tags only; watch Serial for hits.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static uint8_t kSector = 0;  // EDIT ME (0-15)

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Classic Keyscan (web: /nfc keys scan)");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.printf("Scanning sector %u — place Classic tag...\n", kSector);
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  char keyHex[13] = {0};
  if (!Nfc.scanClassicKey(kSector, keyHex, sizeof(keyHex))) {
    return;
  }
  Serial.printf("Found key for sector %u: %s\n", kSector, keyHex);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(2500);
  RgbRing.setPattern(RingPattern::Scanning);
}

/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC Type 2 dump
 * FULL FIRMWARE: /nfc Dump
 *
 * WHAT IT DOES:
 * Reads NTAG/Ultralight pages and prints hex dump to Serial.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Use your own writable Type 2 tag; dump prints once per tag.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — NFC Dump Type 2");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }

  RgbRing.begin(40);
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Place an NTAG / Ultralight tag...");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  NfcDumpData dump;
  if (!Nfc.dumpType2Tag(dump, 45, 1500) || !dump.success) {
    if (dump.error[0]) {
      Serial.printf("Dump error: %s\n", dump.error);
      delay(1000);
    }
    return;
  }

  Serial.printf("UID: %s  pages=%u\n", dump.tag.uidHex, dump.pageCount);
  for (uint8_t p = 0; p < dump.pageCount; p++) {
    Serial.printf("P%02u: %02X %02X %02X %02X\n",
                  p, dump.pages[p][0], dump.pages[p][1],
                  dump.pages[p][2], dump.pages[p][3]);
  }
  Serial.println("--------");

  RgbRing.setPattern(RingPattern::Success);
  delay(2000);
  RgbRing.setPattern(RingPattern::Scanning);
}

/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC clone Type 2
 * FULL FIRMWARE: /nfc Clone
 *
 * WHAT IT DOES:
 * Restores Type 2 content from a prior dump pattern (lab).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Follow Serial prompts; only on tags you own.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kLabPin = "1234";  // EDIT ME
static NfcDumpData gDump;
static bool gHaveDump = false;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Clone Type 2 (web: /nfc clone)");
  Serial.println("Authorized lab use only.");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.println("Step 1: place SOURCE tag to dump...");
  RgbRing.setPattern(RingPattern::Scanning);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();

  if (!gHaveDump) {
    if (!Nfc.dumpType2Tag(gDump, 45, 1500) || !gDump.success) {
      return;
    }
    gHaveDump = true;
    Serial.printf("Dumped UID=%s pages=%u\n", gDump.tag.uidHex, gDump.pageCount);
    Serial.printf("Step 2: type lab PIN (%s) then Enter...\n", kLabPin);
    Buzzer.playSuccess();
    return;
  }

  if (!Serial.available()) {
    return;
  }
  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin != kLabPin) {
    Serial.println("PIN rejected. Try again.");
    Buzzer.playError();
    return;
  }

  Serial.println("Step 3: place DESTINATION tag...");
  RgbRing.setPattern(RingPattern::Scanning);
  while (true) {
    RgbRing.update();
    char err[64] = {0};
    // restore from page 4 (user area); HAL handles bounds
    if (Nfc.restoreType2Pages(gDump, 4, err, sizeof(err))) {
      Serial.println("OK — clone/restore finished.");
      Buzzer.playNfcCloneOk();
      RgbRing.setPattern(RingPattern::Success);
      gHaveDump = false;
      delay(2500);
      Serial.println("Place a new SOURCE tag to dump again...");
      RgbRing.setPattern(RingPattern::Scanning);
      break;
    }
    if (err[0]) {
      Serial.printf("Waiting/fail: %s\n", err);
      delay(600);
    }
  }
}

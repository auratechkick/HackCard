/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Mifare Classic clone
 * FULL FIRMWARE: /nfc Classic clone
 *
 * WHAT IT DOES:
 * Lab clone flow from dump data (authorized use only).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit keys/dump source as marked in sketch.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kLabPin = "1234";           // EDIT ME
static const char* kKeyHex = "FFFFFFFFFFFF";  // EDIT ME

static NfcClassicDump gDump;
static bool gHaveDump = false;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Classic Clone (web: /nfc classic clone)");
  Serial.println("Authorized lab use only.");
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  Serial.println("Place SOURCE Classic tag...");
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

  if (!gHaveDump) {
    if (!Nfc.dumpClassicTag(key, gDump, 16, 2500) || !gDump.success) {
      if (gDump.error[0]) Serial.printf("%s\n", gDump.error);
      delay(500);
      return;
    }
    gHaveDump = true;
    Serial.printf("Dumped UID=%s\nType PIN (%s) then Enter...\n", gDump.tag.uidHex, kLabPin);
    Buzzer.playSuccess();
    return;
  }

  if (!Serial.available()) return;
  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin != kLabPin) {
    Serial.println("PIN rejected.");
    Buzzer.playError();
    return;
  }

  Serial.println("Place DESTINATION Classic tag...");
  while (true) {
    RgbRing.update();
    char e2[64] = {0};
    if (Nfc.cloneClassicDump(gDump, key, e2, sizeof(e2))) {
      Serial.println("OK — classic clone finished.");
      Buzzer.playNfcCloneOk();
      RgbRing.setPattern(RingPattern::Success);
      gHaveDump = false;
      delay(2500);
      Serial.println("Place a new SOURCE tag...");
      RgbRing.setPattern(RingPattern::Scanning);
      break;
    }
    if (e2[0]) {
      Serial.printf("Waiting/fail: %s\n", e2);
      delay(700);
    }
  }
}

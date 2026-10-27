/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: NFC write vCard
 * FULL FIRMWARE: /nfc Write vCard / profile
 *
 * WHAT IT DOES:
 * Encodes contact fields into an NDEF vCard on Type 2.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit name/email/phone; write to your tag.
 *
 * LIBRARIES: HackCard, Adafruit PN532, Adafruit BusIO
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>

static const char* kName = "Alex Rivera";
static const char* kTitle = "Security Researcher";
static const char* kOrg = "HackCard Lab";
static const char* kEmail = "alex@example.com";
static const char* kPhone = "+1 555 0100";
static const char* kUrl = "https://auratechkick.github.io/HackCard/";

String buildVcard() {
  String v;
  v += "BEGIN:VCARD\nVERSION:3.0\n";
  v += "FN:"; v += kName; v += "\n";
  v += "TITLE:"; v += kTitle; v += "\n";
  v += "ORG:"; v += kOrg; v += "\n";
  v += "EMAIL:"; v += kEmail; v += "\n";
  v += "TEL:"; v += kPhone; v += "\n";
  v += "URL:"; v += kUrl; v += "\n";
  v += "END:VCARD\n";
  return v;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — NFC Write vCard (web: /nfc write vcard)");
  Serial.printf("Contact: %s <%s>\n", kName, kEmail);

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not found");
    while (true) delay(1000);
  }
  RgbRing.begin(50);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Place a writable Type 2 tag...");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  String vcard = buildVcard();
  char err[64] = {0};
  if (!Nfc.writeNdefVcard(vcard.c_str(), err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Fail: %s\n", err);
      Buzzer.playError();
      RgbRing.setPattern(RingPattern::Error);
      delay(1200);
      RgbRing.setPattern(RingPattern::Scanning);
    }
    return;
  }
  Serial.println("OK — vCard written. Tap with a phone to save contact.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
  delay(3000);
  RgbRing.setPattern(RingPattern::Scanning);
}

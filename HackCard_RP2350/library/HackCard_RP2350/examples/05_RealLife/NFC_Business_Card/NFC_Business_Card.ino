/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: NFC business card
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * Edit the fields, upload, then tap a blank Type 2 tag you own.
 * The phone that scans the tag can save the contact. There is no BLE share on RP2350.
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";
static const char* kTitle = "Security Researcher";
static const char* kCompany = "Your Company";
static const char* kEmail = "you@example.com";
static const char* kUrl = "https://example.com";

static bool gWritten = false;

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Business Card");
  Serial.printf("Contact: %s <%s>\n", kName, kEmail);

  RgbRing.begin(50);
  Buzzer.begin(true);

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }

  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Place a writable Type 2 tag on the coil...");
}

void loop() {
  RgbRing.update();
  if (gWritten) {
    return;
  }

  char vcard[320];
  snprintf(vcard, sizeof(vcard),
           "BEGIN:VCARD\r\n"
           "VERSION:3.0\r\n"
           "FN:%s\r\n"
           "ORG:%s\r\n"
           "TITLE:%s\r\n"
           "EMAIL:%s\r\n"
           "URL:%s\r\n"
           "END:VCARD\r\n",
           kName, kCompany, kTitle, kEmail, kUrl);

  char err[64] = {0};
  if (!Nfc.writeNdefVcard(vcard, err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Write failed: %s\n", err);
      RgbRing.setPattern(RingPattern::Error);
      Buzzer.playError();
      delay(1200);
      RgbRing.setPattern(RingPattern::Scanning);
    }
    return;
  }

  gWritten = true;
  Serial.println("Business card written. Scan the tag with a phone.");
  Buzzer.playNfcWriteOk();
  RgbRing.setPattern(RingPattern::Success);
}

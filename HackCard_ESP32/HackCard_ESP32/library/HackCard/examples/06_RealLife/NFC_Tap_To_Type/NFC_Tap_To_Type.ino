/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: NFC tap types a check-in line
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Types into the sheet you focus on this PC. Read badges you are allowed to check in.
 *
 * Focus a spreadsheet or text file. Tap a badge. The card types:
 *   CHECKIN,<uid>
 * and presses Enter, so each tap is one new row.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” NFC tap to type");
  Serial.println("Focus a spreadsheet, then tap badges.");

  UsbHid.begin();
  RgbRing.begin(45);
  Buzzer.begin(true);

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }

  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Ready.");
}

void loop() {
  RgbRing.update();

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 400) || !tag.detected) {
    return;
  }

  char line[80];
  snprintf(line, sizeof(line), "CHECKIN,%s", tag.uidHex);
  Serial.println(line);
  Host.typeLine(line);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(900);
  RgbRing.setPattern(RingPattern::Scanning);
}


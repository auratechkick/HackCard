/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Erase user data on a Type 2 tag you own
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Clears the NDEF area on a tag you own. The tag can be written again.
 *
 * Type ERASE and press Enter, then hold the tag on the coil.
 * This does not set the permanent lock bits. Use NFC_Kill_Type2 for that.
 */

#include <HackCardRP2350.h>

static bool gArmed = false;
static char gLine[24];
static size_t gLineLen = 0;

bool readLine() {
  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      gLine[gLineLen] = '\0';
      gLineLen = 0;
      return true;
    }
    if (gLineLen + 1 < sizeof(gLine)) {
      gLine[gLineLen++] = c;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC erase user data");
  Serial.println("Type ERASE and press Enter, then place a tag you own.");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    while (true) delay(1000);
  }
  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  RgbRing.update();

  if (!gArmed) {
    if (!readLine()) {
      return;
    }
    if (strcmp(gLine, "ERASE") != 0) {
      Serial.println("Type ERASE to arm.");
      return;
    }
    gArmed = true;
    Serial.println("Armed. Hold the tag on the coil.");
    RgbRing.setPattern(RingPattern::Scanning);
    return;
  }

  char err[64] = {0};
  if (!Nfc.eraseType2UserData(err, sizeof(err))) {
    if (err[0]) {
      Serial.printf("Erase failed: %s\n", err);
      Buzzer.playError();
      delay(800);
    }
    return;
  }

  Serial.println("User area cleared. You can write this tag again.");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  gArmed = false;
  delay(1200);
  Serial.println("Type ERASE to clear another tag.");
  RgbRing.setPattern(RingPattern::Idle);
}

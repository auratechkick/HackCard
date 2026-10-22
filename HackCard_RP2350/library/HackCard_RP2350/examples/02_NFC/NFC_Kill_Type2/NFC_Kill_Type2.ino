/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Permanently lock a Type 2 tag you own
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Irreversible. Sacrificial NTAG / Ultralight tags you own only.
 *         Do not use this on payment cards or badges you do not own.
 *
 * 1. Type the lab PIN and press Enter.
 * 2. Place one tag on the coil.
 * 3. Read the UID on the serial monitor.
 * 4. Type YES and press Enter to lock that tag.
 * Anything else cancels. The sketch disarms after one tag.
 */

#include <HackCardRP2350.h>

static const char* kLabPin = "1234";

enum class KillStep : uint8_t { NeedPin, NeedTag, NeedYes };

static KillStep gStep = KillStep::NeedPin;
static char gUid[64];
static char gLine[32];
static size_t gLineLen = 0;

bool readLine(char* dest, size_t destSize) {
  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      dest[gLineLen] = '\0';
      gLineLen = 0;
      return true;
    }
    if (gLineLen + 1 < destSize) {
      dest[gLineLen++] = c;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC Kill Type 2");
  Serial.println("Permanent lock. Use a spare Type 2 tag you own.");
  Serial.printf("Type PIN (%s) and press Enter.\n", kLabPin);

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

  if (gStep == KillStep::NeedPin) {
    if (!readLine(gLine, sizeof(gLine))) {
      return;
    }
    if (strcmp(gLine, kLabPin) != 0) {
      Serial.println("PIN rejected.");
      Buzzer.playError();
      return;
    }
    gStep = KillStep::NeedTag;
    gUid[0] = '\0';
    Serial.println("Armed. Place the spare tag on the coil.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  if (gStep == KillStep::NeedTag) {
    NfcTagInfo tag;
    if (!Nfc.readTag(tag, 300) || !tag.detected) {
      return;
    }
    strncpy(gUid, tag.uidHex, sizeof(gUid) - 1);
    gUid[sizeof(gUid) - 1] = '\0';
    Serial.printf("UID %s\n", gUid);
    Serial.println("Type YES to permanently lock this tag, or NO to cancel.");
    Buzzer.playTune(BuzzerTuneId::Warning);
    gStep = KillStep::NeedYes;
    return;
  }

  if (!readLine(gLine, sizeof(gLine))) {
    return;
  }
  if (strcmp(gLine, "YES") != 0) {
    Serial.println("Cancelled. Type the PIN to arm again.");
    gStep = KillStep::NeedPin;
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  char err[64] = {0};
  if (!Nfc.killType2Tag(err, sizeof(err))) {
    Serial.printf("Lock failed: %s\n", err[0] ? err : "tag moved");
    Buzzer.playError();
    RgbRing.setPattern(RingPattern::Error);
  } else {
    Serial.printf("Locked %s. User memory is closed.\n", gUid);
    Buzzer.playNfcKillOk();
    RgbRing.setPattern(RingPattern::Success);
  }
  gStep = KillStep::NeedPin;
  delay(1500);
  Serial.println("Type the PIN to arm another lock.");
  RgbRing.setPattern(RingPattern::Idle);
}

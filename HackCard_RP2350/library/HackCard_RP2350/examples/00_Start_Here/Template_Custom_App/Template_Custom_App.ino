/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Template — build your own app
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK
 * Serial Monitor: 115200
 *
 * Short press = status blink. NFC tap prints the UID when enabled.
 * This board has no Wi-Fi. Delete the blocks you do not need.
 */

#include <HackCardRP2350.h>

static const char* kOwnerName = "Your Name";
static const bool kUseNfc = true;

void onBadgeTap(const NfcTagInfo& tag) {
  Serial.printf("[app] Tap from %s\n", tag.uidHex);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Template");
  Serial.printf("Owner: %s\n", kOwnerName);

  RgbRing.begin(55);
  Buzzer.begin(true);
  Buttons.begin();
  RgbRing.setPattern(RingPattern::BootAnim);

  if (kUseNfc && !Nfc.begin()) {
    Serial.println("NFC not found (continuing without it)");
  } else if (kUseNfc) {
    Serial.printf("NFC: %s\n", Nfc.getChipInfo());
  }

  delay(600);
  RgbRing.setPattern(RingPattern::Idle);
  Serial.println("Ready. Short press = blink. Tap a tag if NFC is on.");
}

void loop() {
  RgbRing.update();
  Buttons.update();

  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    RgbRing.setPattern(RingPattern::Success);
    Buzzer.playSuccess();
    delay(200);
    RgbRing.setPattern(RingPattern::Idle);
  }

  if (!kUseNfc || !Nfc.isReady()) {
    return;
  }

  NfcTagInfo tag;
  if (Nfc.readTag(tag, 150) && tag.detected) {
    onBadgeTap(tag);
    delay(800);
    RgbRing.setPattern(RingPattern::Idle);
  }
}

/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Tap one enrolled tag to lock this PC
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Locks only the computer this board is plugged into.
 *
 * The first tap enrolls the tag and does not lock.
 * Later taps of that same tag lock the screen.
 * A different tag is ignored.
 * Long-press the button to forget the tag and enroll a new one.
 * w / m / l selects Windows, macOS, or Linux.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static char gOwner[64];
static bool gHaveOwner = false;
static char gLastUid[64];
static uint32_t gLastMs = 0;

void showOs() {
  Serial.printf("OS: %s\n", Host.osName());
  if (gHaveOwner) {
    Serial.printf("Owner tag: %s\n", gOwner);
  } else {
    Serial.println("No owner tag yet. Tap once to enroll.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” NFC tap to lock");
  Serial.println("First tap enrolls. Later taps lock this PC.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(45);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  RgbRing.setPattern(RingPattern::Scanning);
  showOs();
}

void loop() {
  RgbRing.update();
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') Host.setOs(HackOs::Windows);
    else if (cmd == 'm' || cmd == 'M') Host.setOs(HackOs::MacOS);
    else if (cmd == 'l' || cmd == 'L') Host.setOs(HackOs::Linux);
    showOs();
  }

  if (Buttons.consumeGesture() == ButtonGesture::Long) {
    gHaveOwner = false;
    gOwner[0] = '\0';
    Serial.println("Owner tag cleared. Tap a tag to enroll.");
    Buzzer.playTune(BuzzerTuneId::Warning);
    RgbRing.setPattern(RingPattern::Scanning);
  }

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 250) || !tag.detected) {
    return;
  }
  if (strcmp(gLastUid, tag.uidHex) == 0 && millis() - gLastMs < 2500) {
    return;
  }
  strncpy(gLastUid, tag.uidHex, sizeof(gLastUid) - 1);
  gLastMs = millis();

  if (!gHaveOwner) {
    strncpy(gOwner, tag.uidHex, sizeof(gOwner) - 1);
    gOwner[sizeof(gOwner) - 1] = '\0';
    gHaveOwner = true;
    Serial.printf("Enrolled %s. Tap it again to lock.\n", gOwner);
    Buzzer.playSuccess();
    RgbRing.setPattern(RingPattern::Success);
    delay(500);
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  if (strcmp(tag.uidHex, gOwner) != 0) {
    Serial.printf("Ignored %s\n", tag.uidHex);
    Buzzer.playError();
    RgbRing.setPattern(RingPattern::Error);
    delay(300);
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  Serial.printf("Locking (%s)\n", Host.osName());
  RgbRing.setPattern(RingPattern::Error);
  Buzzer.playTune(BuzzerTuneId::Alert);
  delay(400);
  Host.lockScreen();
}


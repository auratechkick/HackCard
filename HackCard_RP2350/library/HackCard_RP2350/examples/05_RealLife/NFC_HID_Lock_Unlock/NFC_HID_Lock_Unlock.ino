/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: One tag locks and unlocks this PC
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Your computer, your tag, your password. The password stays in this sketch.
 *
 * Edit kPassword. "change-me" will lock, but it will not unlock.
 * The first tap enrolls the tag and does not lock.
 * Each later tap toggles: lock, then unlock, then lock.
 * Unlock types the password into the lock screen and presses Enter.
 * Use the same PIN or password you type at that screen.
 * w / m / l selects Windows, macOS, or Linux.
 * Long-press forgets the tag.
 */

#include <HackCardRP2350.h>

static const char* kPassword = "change-me";

static char gOwner[64];
static bool gHaveOwner = false;
static bool gLocked = false;
static char gLastUid[64];
static uint32_t gLastMs = 0;

void paint() {
  if (!gHaveOwner) {
    RgbRing.setPattern(RingPattern::Scanning);
  } else if (gLocked) {
    RgbRing.showColor(180, 20, 20);
  } else {
    RgbRing.showColor(0, 150, 40);
  }
}

void showStatus() {
  Serial.printf("OS: %s\n", Host.osName());
  if (!gHaveOwner) {
    Serial.println("Tap a tag once to enroll it.");
    return;
  }
  Serial.printf("Tag: %s\n", gOwner);
  Serial.println(gLocked ? "State: locked" : "State: unlocked");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC lock / unlock");
  Serial.println("Enroll with one tap. Later taps toggle lock and unlock.");
  if (strcmp(kPassword, "change-me") == 0) {
    Serial.println("Unlock is off until you edit kPassword.");
  }

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(45);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  if (HackSd.begin() && HackSd.exists("/nfc/owner-uid.txt")) {
    if (HackSd.readText("/nfc/owner-uid.txt", gOwner, sizeof(gOwner))) {
      gHaveOwner = gOwner[0] != '\0';
    }
  }
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  paint();
  showStatus();
}

void rememberOwner(const char* uid) {
  strncpy(gOwner, uid, sizeof(gOwner) - 1);
  gOwner[sizeof(gOwner) - 1] = '\0';
  gHaveOwner = true;
  gLocked = false;
  if (HackSd.isMounted()) {
    HackSd.makeFolder("/nfc");
    HackSd.writeText("/nfc/owner-uid.txt", gOwner);
  }
}

void loop() {
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') Host.setOs(HackOs::Windows);
    else if (cmd == 'm' || cmd == 'M') Host.setOs(HackOs::MacOS);
    else if (cmd == 'l' || cmd == 'L') Host.setOs(HackOs::Linux);
    showStatus();
  }

  if (Buttons.consumeGesture() == ButtonGesture::Long) {
    gHaveOwner = false;
    gOwner[0] = '\0';
    gLocked = false;
    Serial.println("Tag forgotten.");
    Buzzer.playTune(BuzzerTuneId::Warning);
    paint();
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
    rememberOwner(tag.uidHex);
    Serial.printf("Enrolled %s. Next tap locks.\n", gOwner);
    Buzzer.playSuccess();
    paint();
    return;
  }
  if (strcmp(tag.uidHex, gOwner) != 0) {
    Serial.printf("Ignored %s\n", tag.uidHex);
    Buzzer.playError();
    return;
  }

  if (!gLocked) {
    Serial.println("Locking");
    Host.lockScreen();
    gLocked = true;
    Buzzer.playTune(BuzzerTuneId::Alert);
  } else if (Host.unlockScreen(kPassword)) {
    Serial.println("Unlock sent");
    gLocked = false;
    Buzzer.playSuccess();
  }
  paint();
}

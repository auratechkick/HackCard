/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Walk-away lock, tap-back unlock
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Your computer and a tag you enroll. The password stays in this sketch.
 *
 * Leave the enrolled tag on the coil while you are at the desk (ring green).
 * If the tag is gone for kAwayMs, the PC locks once (ring amber).
 * When that tag comes back, the password is typed into the lock screen.
 * Edit kPassword. "change-me" still locks, but it will not unlock.
 * The first tap only enrolls. w / m / l selects the OS.
 * If the ring goes amber while the tag is still sitting there, lift it and set it down again.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static const char* kPassword = "change-me";
static const uint32_t kAwayMs = 8000;

static char gOwner[64];
static bool gHaveOwner = false;
static bool gPresent = false;
static bool gLocked = false;
static uint32_t gLastSeen = 0;

void paint() {
  if (!gHaveOwner) {
    RgbRing.setPattern(RingPattern::Scanning);
  } else if (gPresent) {
    RgbRing.showColor(0, 150, 40);
  } else {
    RgbRing.showColor(220, 120, 0);
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” desk presence");
  Serial.printf("Away lock after %lu seconds without the tag.\n", kAwayMs / 1000);
  if (strcmp(kPassword, "change-me") == 0) {
    Serial.println("Unlock is off until you edit kPassword.");
  }

  UsbHid.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  if (HackSd.begin() && HackSd.exists("/nfc/owner-uid.txt")) {
    if (HackSd.readText("/nfc/owner-uid.txt", gOwner, sizeof(gOwner))) {
      gHaveOwner = gOwner[0] != '\0';
      Serial.printf("Loaded tag %s\n", gOwner);
    }
  }
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  paint();
  Serial.println(gHaveOwner ? "Waiting for your tag." : "Tap a tag once to enroll.");
}

void loop() {
  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') Host.setOs(HackOs::Windows);
    else if (cmd == 'm' || cmd == 'M') Host.setOs(HackOs::MacOS);
    else if (cmd == 'l' || cmd == 'L') Host.setOs(HackOs::Linux);
    Serial.printf("OS: %s\n", Host.osName());
  }

  NfcTagInfo tag;
  const bool seen = Nfc.readTag(tag, 150) && tag.detected;
  const uint32_t now = millis();

  if (seen && !gHaveOwner) {
    strncpy(gOwner, tag.uidHex, sizeof(gOwner) - 1);
    gOwner[sizeof(gOwner) - 1] = '\0';
    gHaveOwner = true;
    gPresent = true;
    gLastSeen = now;
    if (HackSd.isMounted()) {
      HackSd.makeFolder("/nfc");
      HackSd.writeText("/nfc/owner-uid.txt", gOwner);
    }
    Serial.printf("Enrolled %s. Leave it on the coil while you are here.\n", gOwner);
    Buzzer.playSuccess();
    paint();
    return;
  }

  if (seen && strcmp(tag.uidHex, gOwner) == 0) {
    if (!gPresent) {
      gPresent = true;
      Serial.println("Tag is back.");
      if (gLocked && Host.unlockScreen(kPassword)) {
        gLocked = false;
        Serial.println("Unlock sent");
        Buzzer.playSuccess();
      }
      paint();
    }
    gLastSeen = now;
    return;
  }

  if (gHaveOwner && gPresent && gLastSeen != 0 && now - gLastSeen > kAwayMs) {
    gPresent = false;
    Serial.println("Tag gone. Locking.");
    Host.lockScreen();
    gLocked = true;
    Buzzer.playTune(BuzzerTuneId::Alert);
    paint();
  }
}


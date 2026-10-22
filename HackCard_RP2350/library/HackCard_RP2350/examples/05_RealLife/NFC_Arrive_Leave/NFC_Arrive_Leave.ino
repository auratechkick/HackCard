/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Arrive tag and leave tag
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Your computer and tags you enroll. The password stays in this sketch.
 *
 * learn owner   then tap — that tag unlocks this PC and opens kStartUrl
 * learn away    then tap — that tag mutes audio and locks the screen
 * list / forget
 * w / m / l selects Windows, macOS, or Linux
 *
 * Edit kPassword and kStartUrl. "change-me" will not type a password.
 * Bindings are saved in /nfc/arrive-leave.txt when a card is inserted.
 */

#include <HackCardRP2350.h>

static const char* kPassword = "change-me";
static const char* kStartUrl = "https://example.com";

enum class DeskRole : uint8_t { None, Owner, Away };

static char gOwner[48];
static char gAway[48];
static bool gHaveOwner = false;
static bool gHaveAway = false;
static DeskRole gLearn = DeskRole::None;
static char gLastUid[48];
static uint32_t gLastMs = 0;
static char gLine[32];
static size_t gLineLen = 0;

void saveRoles() {
  if (!HackSd.isMounted()) {
    return;
  }
  HackSd.makeFolder("/nfc");
  char body[120];
  snprintf(body, sizeof(body), "owner %s\naway %s\n", gHaveOwner ? gOwner : "-", gHaveAway ? gAway : "-");
  HackSd.writeText("/nfc/arrive-leave.txt", body);
}

void loadRoles() {
  if (!HackSd.isMounted() || !HackSd.exists("/nfc/arrive-leave.txt")) {
    return;
  }
  char text[120];
  if (!HackSd.readText("/nfc/arrive-leave.txt", text, sizeof(text))) {
    return;
  }
  char kind[16];
  char uid[48];
  char* cursor = text;
  while (*cursor) {
    char* end = strchr(cursor, '\n');
    if (end) {
      *end = '\0';
    }
    if (sscanf(cursor, "%15s %47s", kind, uid) == 2 && strcmp(uid, "-") != 0) {
      if (strcmp(kind, "owner") == 0) {
        strncpy(gOwner, uid, sizeof(gOwner) - 1);
        gHaveOwner = true;
      } else if (strcmp(kind, "away") == 0) {
        strncpy(gAway, uid, sizeof(gAway) - 1);
        gHaveAway = true;
      }
    }
    if (!end) {
      break;
    }
    cursor = end + 1;
  }
}

bool takeLine() {
  while (Serial.available()) {
    const char c = static_cast<char>(Serial.read());
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      gLine[gLineLen] = '\0';
      gLineLen = 0;
      return gLine[0] != '\0';
    }
    if (gLineLen + 1 < sizeof(gLine)) {
      gLine[gLineLen++] = c;
    }
  }
  return false;
}

void listRoles() {
  Serial.printf("OS: %s\n", Host.osName());
  Serial.printf("Owner: %s\n", gHaveOwner ? gOwner : "(none)");
  Serial.printf("Away:  %s\n", gHaveAway ? gAway : "(none)");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — arrive / leave tags");
  Serial.println("learn owner | learn away | list | forget | w m l");
  if (strcmp(kPassword, "change-me") == 0) {
    Serial.println("Unlock is off until you edit kPassword.");
  }

  UsbHid.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  if (HackSd.begin()) {
    loadRoles();
  }
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  RgbRing.setPattern(RingPattern::Idle);
  listRoles();
}

void loop() {
  RgbRing.update();

  if (takeLine()) {
    if (strcmp(gLine, "w") == 0) Host.setOs(HackOs::Windows);
    else if (strcmp(gLine, "m") == 0) Host.setOs(HackOs::MacOS);
    else if (strcmp(gLine, "l") == 0) Host.setOs(HackOs::Linux);
    else if (strcmp(gLine, "list") == 0) {
      listRoles();
    } else if (strcmp(gLine, "forget") == 0) {
      gHaveOwner = false;
      gHaveAway = false;
      gLearn = DeskRole::None;
      saveRoles();
      Serial.println("Cleared.");
    } else if (strcmp(gLine, "learn owner") == 0) {
      gLearn = DeskRole::Owner;
      Serial.println("Tap the arrive tag.");
      RgbRing.setPattern(RingPattern::Scanning);
    } else if (strcmp(gLine, "learn away") == 0) {
      gLearn = DeskRole::Away;
      Serial.println("Tap the leave tag.");
      RgbRing.setPattern(RingPattern::Scanning);
    } else {
      Serial.println("learn owner | learn away | list | forget");
    }
    if (gLine[0] == 'w' || gLine[0] == 'm' || gLine[0] == 'l') {
      listRoles();
    }
  }

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 200) || !tag.detected) {
    return;
  }
  if (strcmp(gLastUid, tag.uidHex) == 0 && millis() - gLastMs < 2500) {
    return;
  }
  strncpy(gLastUid, tag.uidHex, sizeof(gLastUid) - 1);
  gLastMs = millis();

  if (gLearn == DeskRole::Owner) {
    strncpy(gOwner, tag.uidHex, sizeof(gOwner) - 1);
    gHaveOwner = true;
    gLearn = DeskRole::None;
    saveRoles();
    Serial.printf("Owner tag %s\n", gOwner);
    Buzzer.playSuccess();
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }
  if (gLearn == DeskRole::Away) {
    strncpy(gAway, tag.uidHex, sizeof(gAway) - 1);
    gHaveAway = true;
    gLearn = DeskRole::None;
    saveRoles();
    Serial.printf("Away tag %s\n", gAway);
    Buzzer.playSuccess();
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  if (gHaveOwner && strcmp(tag.uidHex, gOwner) == 0) {
    Serial.println("Arrive");
    RgbRing.showColor(0, 150, 40);
    if (Host.unlockScreen(kPassword)) {
      delay(1200);
      Host.openUrl(kStartUrl);
    }
    Buzzer.playSuccess();
    return;
  }
  if (gHaveAway && strcmp(tag.uidHex, gAway) == 0) {
    Serial.println("Leave");
    UsbHid.mute();
    delay(200);
    Host.lockScreen();
    RgbRing.showColor(220, 120, 0);
    Buzzer.playTune(BuzzerTuneId::Alert);
    return;
  }
  Serial.printf("Unknown tag %s\n", tag.uidHex);
  Buzzer.playError();
}

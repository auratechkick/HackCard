/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Different tags trigger different HID actions
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Actions run on the PC this board is plugged into. Enroll tags you own.
 *
 * Teach a tag, then tap it:
 *   learn lock     tap locks the screen
 *   learn shot     tap takes a screenshot
 *   learn url      tap opens kUrl
 *   learn sign     tap types your name and site into the focused window
 *   learn mute     tap mutes the speaker
 *   list           show the enrolled tags
 *   forget         clear every tag
 *   w / m / l      Windows, macOS, or Linux
 *
 * Bindings are kept in RAM and, when a card is inserted, in /nfc/hid-tags.txt.
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";
static const char* kUrl = "https://example.com";

enum class TagJob : uint8_t { None, Lock, Shot, Url, Sign, Mute };

struct TagBinding {
  char uid[48];
  TagJob job;
};

static const uint8_t kSlots = 6;
static TagBinding gTags[kSlots];
static uint8_t gCount = 0;
static TagJob gLearn = TagJob::None;
static char gLastUid[48];
static uint32_t gLastMs = 0;
static char gLine[48];
static size_t gLineLen = 0;

const char* jobName(TagJob job) {
  switch (job) {
    case TagJob::Lock:
      return "lock";
    case TagJob::Shot:
      return "shot";
    case TagJob::Url:
      return "url";
    case TagJob::Sign:
      return "sign";
    case TagJob::Mute:
      return "mute";
    default:
      return "none";
  }
}

TagJob jobFromName(const char* name) {
  if (strcmp(name, "lock") == 0) return TagJob::Lock;
  if (strcmp(name, "shot") == 0) return TagJob::Shot;
  if (strcmp(name, "url") == 0) return TagJob::Url;
  if (strcmp(name, "sign") == 0) return TagJob::Sign;
  if (strcmp(name, "mute") == 0) return TagJob::Mute;
  return TagJob::None;
}

void saveTags() {
  if (!HackSd.isMounted()) {
    return;
  }
  HackSd.makeFolder("/nfc");
  HackSd.writeText("/nfc/hid-tags.txt", "");
  for (uint8_t i = 0; i < gCount; i++) {
    char line[64];
    snprintf(line, sizeof(line), "%s %s", jobName(gTags[i].job), gTags[i].uid);
    HackSd.appendLine("/nfc/hid-tags.txt", line);
  }
}

void loadTags() {
  if (!HackSd.isMounted() || !HackSd.exists("/nfc/hid-tags.txt")) {
    return;
  }
  char text[512];
  if (!HackSd.readText("/nfc/hid-tags.txt", text, sizeof(text))) {
    return;
  }
  gCount = 0;
  char* cursor = text;
  while (*cursor && gCount < kSlots) {
    char* end = strchr(cursor, '\n');
    if (end) {
      *end = '\0';
    }
    char job[16] = {0};
    char uid[48] = {0};
    if (sscanf(cursor, "%15s %47s", job, uid) == 2) {
      const TagJob parsed = jobFromName(job);
      if (parsed != TagJob::None) {
        gTags[gCount].job = parsed;
        strncpy(gTags[gCount].uid, uid, sizeof(gTags[gCount].uid) - 1);
        gCount++;
      }
    }
    if (!end) {
      break;
    }
    cursor = end + 1;
  }
  Serial.printf("Loaded %u tag binding(s)\n", gCount);
}

void listTags() {
  if (gCount == 0) {
    Serial.println("No tags enrolled.");
    return;
  }
  for (uint8_t i = 0; i < gCount; i++) {
    Serial.printf("  %s  %s\n", jobName(gTags[i].job), gTags[i].uid);
  }
}

int findTag(const char* uid) {
  for (uint8_t i = 0; i < gCount; i++) {
    if (strcmp(gTags[i].uid, uid) == 0) {
      return i;
    }
  }
  return -1;
}

void remember(const char* uid, TagJob job) {
  const int existing = findTag(uid);
  if (existing >= 0) {
    gTags[existing].job = job;
  } else if (gCount < kSlots) {
    strncpy(gTags[gCount].uid, uid, sizeof(gTags[gCount].uid) - 1);
    gTags[gCount].job = job;
    gCount++;
  } else {
    Serial.println("Tag list is full. Type forget first.");
    return;
  }
  saveTags();
  Serial.printf("Enrolled %s for %s\n", uid, jobName(job));
  Buzzer.playSuccess();
}

void runJob(TagJob job) {
  switch (job) {
    case TagJob::Lock:
      Serial.printf("Lock (%s)\n", Host.osName());
      RgbRing.setPattern(RingPattern::Error);
      delay(300);
      Host.lockScreen();
      break;
    case TagJob::Shot:
      Serial.println("Screenshot");
      Host.screenshot();
      RgbRing.setPattern(RingPattern::Success);
      break;
    case TagJob::Url:
      Serial.printf("Open %s\n", kUrl);
      Host.openUrl(kUrl);
      RgbRing.setPattern(RingPattern::Scanning);
      break;
    case TagJob::Sign:
      Host.typeText(kName);
      Host.typeText("  ");
      Host.typeLine(kUrl);
      RgbRing.setPattern(RingPattern::Success);
      break;
    case TagJob::Mute:
      UsbHid.mute();
      Serial.println("Mute");
      RgbRing.showColor(180, 40, 220);
      break;
    default:
      break;
  }
  Buzzer.playSuccess();
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

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — NFC tags to HID");
  Serial.println("learn lock | learn shot | learn url | learn sign | learn mute");
  Serial.println("list | forget | w m l");

  UsbHid.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  if (HackSd.begin()) {
    loadTags();
  }
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  RgbRing.setPattern(RingPattern::Idle);
  Serial.printf("OS: %s\n", Host.osName());
}

void loop() {
  RgbRing.update();

  if (takeLine()) {
    if (strcmp(gLine, "w") == 0) Host.setOs(HackOs::Windows);
    else if (strcmp(gLine, "m") == 0) Host.setOs(HackOs::MacOS);
    else if (strcmp(gLine, "l") == 0) Host.setOs(HackOs::Linux);
    else if (strcmp(gLine, "list") == 0) listTags();
    else if (strcmp(gLine, "forget") == 0) {
      gCount = 0;
      gLearn = TagJob::None;
      saveTags();
      Serial.println("Cleared.");
    } else if (strncmp(gLine, "learn ", 6) == 0) {
      gLearn = jobFromName(gLine + 6);
      if (gLearn == TagJob::None) {
        Serial.println("Use lock, shot, url, sign, or mute.");
      } else {
        Serial.printf("Tap a tag to enroll for %s\n", jobName(gLearn));
        RgbRing.setPattern(RingPattern::Scanning);
      }
    } else {
      Serial.println("learn lock | learn shot | learn url | learn sign | learn mute");
    }
    if (strcmp(gLine, "w") == 0 || strcmp(gLine, "m") == 0 || strcmp(gLine, "l") == 0) {
      Serial.printf("OS: %s\n", Host.osName());
    }
  }

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 200) || !tag.detected) {
    return;
  }
  if (strcmp(gLastUid, tag.uidHex) == 0 && millis() - gLastMs < 2200) {
    return;
  }
  strncpy(gLastUid, tag.uidHex, sizeof(gLastUid) - 1);
  gLastMs = millis();

  if (gLearn != TagJob::None) {
    remember(tag.uidHex, gLearn);
    gLearn = TagJob::None;
    RgbRing.setPattern(RingPattern::Idle);
    return;
  }

  const int slot = findTag(tag.uidHex);
  if (slot < 0) {
    Serial.printf("Unknown tag %s\n", tag.uidHex);
    Buzzer.playError();
    return;
  }
  runJob(gTags[slot].job);
}

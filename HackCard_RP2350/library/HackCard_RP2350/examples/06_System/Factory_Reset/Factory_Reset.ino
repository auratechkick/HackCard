/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Factory reset
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * WHAT IT DOES:
 * Erases saved lab files on the microSD card. Nothing is erased until you
 * hold the user button for 3 seconds or type RESET.
 *
 * CLEARS (SD card, if inserted):
 *   /nfc        enrolled tags, dumps, check-in logs
 *   /ducky      scripts saved on the card
 *   /payloads   saved payload files
 *   /logs       activity logs
 *   /hackcard   lab folder
 *
 * THIS BOARD DOES NOT STORE:
 *   Wi-Fi passwords (no radio)
 *   A settings filesystem (flash is 2MB, no FS)
 *   The profile and lab PIN — those are compiled from
 *   HackCard_RP2350/config/user_config.h
 *   Edit that file and upload HackCard_RP2350.ino to change them.
 *
 * DOES NOT format the whole SD card. Set kWipeSdLabData to false to skip the card.
 * This sketch replaces the serial lab until you upload HackCard_RP2350.ino again.
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard RP2350 -> 06_System -> Factory_Reset
 *   2. Upload. Open Serial Monitor @ 115200. Line ending: Newline.
 *   3. Hold the user button (not BOOT or RUN) for 3 seconds, or type RESET.
 *   4. Upload HackCard_RP2350.ino when Serial says factory reset complete.
 */

#include <HackCardRP2350.h>
#include <SD.h>

static const uint32_t kHoldMs = 3000;
static const bool kWipeSdLabData = true;

static const char* kSdFolders[] = {
    "/hackcard",
    "/nfc",
    "/payloads",
    "/logs",
    "/ducky",
};

static bool gDone = false;

void note(bool ok, const char* step, const char* detail) {
  Serial.printf("  [%s] %s — %s\n", ok ? "OK" : "FAIL", step, detail);
}

bool wipeSdTree(const char* path, int depth, int pass) {
  if (!path || depth > 8 || pass > 8) {
    return false;
  }

  File dir = SD.open(path);
  if (!dir) {
    return true;
  }
  if (!dir.isDirectory()) {
    dir.close();
    return SD.remove(path);
  }

  char names[12][48];
  bool isDir[12];
  int count = 0;
  bool more = false;

  File entry = dir.openNextFile();
  while (entry) {
    const char* raw = entry.name();
    const char* slash = raw ? strrchr(raw, '/') : nullptr;
    const char* base = slash ? slash + 1 : raw;
    const bool directory = entry.isDirectory();
    if (base && base[0] != '\0' && strcmp(base, ".") != 0 && strcmp(base, "..") != 0) {
      if (strlen(base) >= sizeof(names[0])) {
        if (raw && raw[0] == '/' && !directory) {
          SD.remove(raw);
        } else {
          Serial.printf("  skipped long name under %s\n", path);
        }
      } else if (count >= 12) {
        more = true;
      } else {
        strlcpy(names[count], base, sizeof(names[0]));
        isDir[count] = directory;
        count++;
      }
    }
    entry = dir.openNextFile();
  }
  dir.close();

  bool ok = true;
  for (int i = 0; i < count; i++) {
    char child[96];
    if (strcmp(path, "/") == 0) {
      snprintf(child, sizeof(child), "/%s", names[i]);
    } else {
      snprintf(child, sizeof(child), "%s/%s", path, names[i]);
    }
    if (isDir[i]) {
      ok = wipeSdTree(child, depth + 1, 0) && ok;
    } else if (!SD.remove(child)) {
      Serial.printf("  could not remove %s\n", child);
      ok = false;
    }
  }

  if (more) {
    return wipeSdTree(path, depth, pass + 1) && ok;
  }
  if (strcmp(path, "/") != 0 && !SD.rmdir(path) && SD.exists(path)) {
    ok = false;
  }
  return ok;
}

void runFactoryReset() {
  if (gDone) {
    Serial.println("Already reset this boot. Upload HackCard_RP2350.ino, or press RUN to start again.");
    return;
  }

  Serial.println();
  Serial.println("Factory reset running...");
  RgbRing.showColor(80, 40, 0);

  note(true, "Profile and lab PIN", "not stored on the chip — they come from user_config.h");
  note(true, "Wi-Fi passwords", "this board has no radio, so none are saved");

  if (!kWipeSdLabData) {
    note(true, "SD card", "left untouched (kWipeSdLabData is false)");
  } else if (!HackSd.begin()) {
    note(true, "SD card", "no card in the slot — nothing to remove");
  } else {
    bool sdOk = true;
    for (const char* folder : kSdFolders) {
      Serial.printf("  SD %s\n", folder);
      sdOk = wipeSdTree(folder, 0, 0) && sdOk;
    }
    note(sdOk, "SD lab folders",
         sdOk ? "nfc, ducky, payloads, logs, and hackcard removed"
              : "some SD files could not be removed");
  }

  gDone = true;
  RgbRing.showColor(0, 60, 0);
  Buzzer.playSuccess();
  Serial.println();
  Serial.println("Factory reset complete.");
  Serial.println("Upload HackCard_RP2350.ino next.");
  Serial.println("Name, contact details, and lab PIN stay as written in config/user_config.h.");
  Serial.println("Saved tag lists, dumps, scripts, and logs on the card are gone.");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  pinMode(PIN_BUTTON, INPUT_PULLUP);

  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);

  Serial.println();
  Serial.println("HackCard RP2350 — Factory Reset");
  Serial.println("This sketch does not erase anything until you confirm.");
  Serial.println();
  Serial.println("It will clear HackCard folders on the SD card:");
  Serial.println("  /nfc  /ducky  /payloads  /logs  /hackcard");
  Serial.println("Profile text and the lab PIN are in config/user_config.h, not on the card.");
  Serial.println();
  Serial.println("Hold the user button for 3 seconds, or type RESET and press Enter.");
  Serial.println("BOOT and RUN are hardware-only. Use the user button.");
}

void loop() {
  RgbRing.update();

  if (!gDone && digitalRead(PIN_BUTTON) == LOW) {
    const uint32_t started = millis();
    RgbRing.showColor(80, 40, 0);
    while (digitalRead(PIN_BUTTON) == LOW) {
      if (millis() - started >= kHoldMs) {
        runFactoryReset();
        break;
      }
      delay(20);
    }
    if (!gDone) {
      RgbRing.setPattern(RingPattern::Idle);
    }
  }

  while (Serial.available() > 0) {
    static char line[16];
    static uint8_t len = 0;
    const char c = static_cast<char>(Serial.read());
    if (c == '\n' || c == '\r') {
      line[len] = '\0';
      if (strcasecmp(line, "RESET") == 0) {
        runFactoryReset();
      } else if (len > 0) {
        Serial.println("Type RESET to confirm.");
      }
      len = 0;
    } else if (len + 1 < sizeof(line)) {
      line[len++] = c;
    }
  }

  delay(20);
}

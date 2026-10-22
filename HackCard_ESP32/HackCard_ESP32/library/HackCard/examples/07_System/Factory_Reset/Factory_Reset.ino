/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Factory reset
 * FULL FIRMWARE: Settings stored in NVS + /config/hackcard.json
 *
 * WHAT IT DOES:
 * Erases saved device settings so the next full-firmware boot is factory-fresh.
 * Nothing is erased until you hold BOOT for 3 seconds or type RESET.
 *
 * CLEARS:
 *   Wi-Fi AP password, lab PIN, and saved router (STA) password
 *   Personal profile (name, title, company, email, phone, links, bio)
 *   First-run flag, device mode, and other fields in hackcard.json
 *   NVS backup of that config, plus Wi-Fi credentials stored by the radio
 *   On-chip flash lab files (logs, NFC dumps, payloads, training text)
 *   The same lab folders on the microSD card, if a card is inserted
 *
 * DOES NOT:
 *   Change config/user_config.h in the firmware source
 *   Format the whole SD card (only HackCard lab folders)
 *   Keep this sketch as the product. Upload HackCard_ESP32.ino again after.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB) if you use it for the full firmware
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> 07_System -> Factory_Reset
 *   2. Upload. Do not hold BOOT while the board resets (that is download mode).
 *   3. Open Serial Monitor @ 115200 baud. Set line ending to Newline.
 *   4. Hold BOOT for 3 seconds, or type RESET and press Enter.
 *   5. When Serial says factory reset complete, upload HackCard_ESP32.ino.
 *      The card then uses the defaults in config/user_config.h
 *      (AP name, AP password, lab PIN, and profile text in that file).
 *
 * LIBRARIES: HackCard, LittleFS, SD, SPI, WiFi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <LittleFS.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include <Preferences.h>
#include <nvs_flash.h>

static const uint32_t kHoldMs = 3000;
static const bool kWipeSdLabData = true;

static const char* kSdFolders[] = {
    "/config",
    "/logs",
    "/nfc",
    "/wifi",
    "/ble",
    "/payloads",
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

bool clearSavedWifi() {
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();
  delay(100);
  return true;
}

bool clearNvs() {
  Preferences prefs;
  if (prefs.begin("hackcard", false)) {
    prefs.clear();
    prefs.end();
  }

  const esp_err_t erased = nvs_flash_erase();
  const esp_err_t inited = nvs_flash_init();
  return erased == ESP_OK && inited == ESP_OK;
}

bool clearFlashFiles() {
  LittleFS.begin(false);
  const bool formatted = LittleFS.format();
  LittleFS.end();
  return formatted;
}

void runFactoryReset() {
  if (gDone) {
    Serial.println("Already reset this boot. Upload HackCard_ESP32.ino, or press EN to run again.");
    return;
  }

  Serial.println();
  Serial.println("Factory reset running...");
  RgbRing.showColor(80, 40, 0);

  const bool wifiOk = clearSavedWifi();
  note(wifiOk, "Wi-Fi passwords", wifiOk ? "saved network and AP config cleared" : "radio clear reported failure");

  const bool nvsOk = clearNvs();
  note(nvsOk, "NVS settings", nvsOk ? "profile, lab PIN, and AP password backup removed" : "NVS erase failed");

  const bool flashOk = clearFlashFiles();
  note(flashOk, "Flash files", flashOk ? "lab files and /config/hackcard.json removed" : "LittleFS format failed");

  if (!kWipeSdLabData) {
    note(true, "SD card", "left untouched (kWipeSdLabData is false)");
  } else {
    SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    if (!SD.begin(PIN_SD_CS)) {
      note(true, "SD card", "no card in the slot — nothing to remove");
    } else {
      bool sdOk = true;
      for (const char* folder : kSdFolders) {
        Serial.printf("  SD %s\n", folder);
        sdOk = wipeSdTree(folder, 0, 0) && sdOk;
      }
      note(sdOk, "SD lab folders", sdOk ? "config, logs, NFC, Wi-Fi, BLE, payloads removed" : "some SD files could not be removed");
    }
  }

  gDone = true;
  RgbRing.showColor(0, 60, 0);
  Buzzer.playSuccess();
  Serial.println();
  Serial.println("Factory reset complete.");
  Serial.println("Upload HackCard_ESP32.ino next.");
  Serial.println("That firmware starts from config/user_config.h:");
  Serial.println("  AP name and AP password, lab PIN, and the profile text in that file.");
  Serial.println("Saved overrides of those values are gone.");
}

void setup() {
  Serial.begin(115200);
  delay(800);
  pinMode(PIN_BOOT, INPUT_PULLUP);

  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);

  Serial.println();
  Serial.println("HackCard — Factory Reset");
  Serial.println("This sketch does not erase anything until you confirm.");
  Serial.println();
  Serial.println("It will clear:");
  Serial.println("  - AP password, lab PIN, saved Wi-Fi password");
  Serial.println("  - profile (name, email, phone, links, bio)");
  Serial.println("  - other saved settings and lab files on flash");
  Serial.println("  - HackCard folders on the SD card, if one is inserted");
  Serial.println();
  Serial.println("Hold BOOT for 3 seconds, or type RESET and press Enter.");
  Serial.println("Do not hold BOOT during upload — release it, then hold after this text.");
}

void loop() {
  RgbRing.update();

  if (!gDone && digitalRead(PIN_BOOT) == LOW) {
    const uint32_t started = millis();
    RgbRing.showColor(80, 40, 0);
    while (digitalRead(PIN_BOOT) == LOW) {
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

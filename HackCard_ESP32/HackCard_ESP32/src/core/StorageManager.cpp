#include "StorageManager.h"

#include "HcSerial.h"
#include "config/board_config.h"
#include "config/user_config.h"

#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>

StorageManager Storage;

namespace {

String normalizeFlashPath(const char* path) {
  if (!path) {
    return "/";
  }
  String normalized = path;
  if (!normalized.startsWith("/")) {
    normalized = "/" + normalized;
  }
  return normalized;
}

String normalizeSdPath(const char* path) {
  return normalizeFlashPath(path);
}

bool isDeviceConfigPath(const char* path) {
  return path && strncmp(path, "/config/", 8) == 0;
}

bool isWifiDataPath(const char* path) {
  return path && strncmp(path, "/wifi/", 6) == 0;
}

bool isPayloadPath(const char* path) {
  return path && strncmp(path, "/payloads/", 10) == 0;
}

}  // namespace

void StorageManager::begin() {
  _flashReady = mountFlash();
  if (_flashReady) {
    ensureFlashFolders();
  }
  _sdReady = false;
  _sdScanActive = true;
  _sdScanStartMs = millis() + SD_SCAN_DELAY_MS;
  _sdNextTryMs = _sdScanStartMs;
  _tier = StorageTier::FlashOnly;
  snprintf(_statusLine, sizeof(_statusLine), "Storage: flash ready (SD scan deferred for Wi-Fi)");
}

bool StorageManager::probeSd() {
  if (_sdReady) {
    return true;
  }
  if (!mountSd()) {
    snprintf(_statusLine, sizeof(_statusLine), "Storage: flash only (no SD card)");
    return false;
  }
  _sdReady = true;
  _tier = StorageTier::SdFull;
  _sdScanActive = false;
  if (SD_AUTO_CREATE_FOLDERS) {
    ensureSdFolders();
  }
  snprintf(_statusLine, sizeof(_statusLine), "Storage: SD card ready (Full Mode)");
  HcSerial.println("[storage] SD card mounted from file browser");
  return true;
}

bool StorageManager::ensureFlashMounted() {
  if (_flashReady) {
    return true;
  }
  if (mountFlash()) {
    _flashReady = true;
    ensureFlashFolders();
    snprintf(_statusLine, sizeof(_statusLine), "Storage: flash ready (mounted on demand)");
    HcSerial.println("[storage] LittleFS mounted on demand");
    return true;
  }
  HcSerial.println("[storage] LittleFS mount failed — check partition scheme and Tools → ESP32 Sketch Data Upload");
  return false;
}

size_t StorageManager::freeFlashBytes() const {
  if (!_flashReady) {
    return 0;
  }
  const size_t total = LittleFS.totalBytes();
  const size_t used = LittleFS.usedBytes();
  return total > used ? total - used : 0;
}

void StorageManager::loop() {
  if (_sdReady || !_sdScanActive) {
    return;
  }

  if (millis() < _sdNextTryMs) {
    return;
  }

  if (mountSd()) {
    _sdReady = true;
    _tier = StorageTier::SdFull;
    if (SD_AUTO_CREATE_FOLDERS) {
      ensureSdFolders();
    }
    snprintf(_statusLine, sizeof(_statusLine), "Storage: SD card ready (Full Mode)");
    return;
  }

  _sdNextTryMs = millis() + 2000;
}

bool StorageManager::mountFlash() {
  if (!LittleFS.begin(true)) {
    return false;
  }
  return true;
}

bool StorageManager::mountSd() {
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (SD.begin(PIN_SD_CS)) {
    return true;
  }
  return false;
}

void StorageManager::ensureSdFolders() {
  if (!_sdReady || _sdFoldersReady) {
    return;
  }

  // Lab data only — dashboard UI is embedded in firmware (not served from SD /www/).
  const char* folders[] = {
      "/config", "/logs", "/nfc", "/nfc/dumps", "/nfc/exports", "/nfc/exports/dumps", "/nfc/exports/classic",
      "/nfc/exports/reads", "/wifi", "/wifi/exports", "/ble", "/payloads"};

  for (const char* folder : folders) {
    if (!SD.exists(folder)) {
      SD.mkdir(folder);
    }
  }

  _sdFoldersReady = true;
}

void StorageManager::ensureFlashFolders() {
  if (!_flashReady) {
    return;
  }

  // LittleFS: lab data + optional /www override for Sketch Data Upload (not used from SD).
  const char* folders[] = {
      "/config", "/logs", "/nfc", "/nfc/dumps", "/nfc/exports", "/nfc/exports/dumps", "/nfc/exports/classic",
      "/nfc/exports/reads", "/wifi", "/wifi/exports", "/ble", "/payloads", "/www", "/www/assets"};

  for (const char* folder : folders) {
    if (!LittleFS.exists(folder)) {
      LittleFS.mkdir(folder);
    }
  }
}

bool StorageManager::ensureDirectory(const char* path) {
  if (!path || path[0] == '\0') {
    return false;
  }

  String dir = normalizeFlashPath(path);
  if (dir.endsWith("/")) {
    dir.remove(dir.length() - 1);
  }

  const int slash = dir.lastIndexOf('/');
  if (slash >= 0 && slash < static_cast<int>(dir.length()) - 1) {
    const String base = dir.substring(slash + 1);
    if (base.indexOf('.') >= 0) {
      dir = dir.substring(0, slash);
    }
  }

  if (dir.isEmpty()) {
    dir = "/";
  }

  bool ok = false;
  if (_flashReady) {
    if (dir == "/" || LittleFS.exists(dir)) {
      ok = true;
    } else if (LittleFS.mkdir(dir)) {
      ok = true;
    }
  }
  if (_sdReady) {
    if (dir == "/" || SD.exists(dir)) {
      ok = true;
    } else if (SD.mkdir(dir)) {
      ok = true;
    }
  }
  return ok;
}

bool StorageManager::readTextFile(const char* path, String& out) {
  out = "";

  // Web UI: LittleFS /www/ optional override; else WebStatusServer serves embedded UI.
  // Do not read /www/ from SD — stale SD copies caused old dashboards after firmware updates.
  if (path && strncmp(path, "/www/", 5) == 0) {
    if (_flashReady && readFromFlash(path, out) && out.length() > 0) {
      return true;
    }
    return false;
  }

  // Device config lives on flash first so a later SD mount cannot override a setup saved before SD was ready.
  if (isDeviceConfigPath(path) || isWifiDataPath(path)) {
    if (_flashReady && readFromFlash(path, out) && out.length() > 0) {
      return true;
    }
    if (_sdReady && readFromSd(path, out) && out.length() > 0) {
      return true;
    }
    return false;
  }

  if (_sdReady && readFromSd(path, out) && out.length() > 0) {
    return true;
  }
  if (_flashReady && readFromFlash(path, out) && out.length() > 0) {
    return true;
  }
  return false;
}

bool StorageManager::writeTextFile(const char* path, const String& data) {
  ensureDirectory(path);

  if (isDeviceConfigPath(path) || isWifiDataPath(path)) {
    bool ok = false;
    if (_flashReady) {
      ok = writeToFlash(path, data) || ok;
    }
    if (_sdReady) {
      ok = writeToSd(path, data) || ok;
    }
    return ok;
  }

  if (isPayloadPath(path)) {
    bool ok = false;
    if (_flashReady) {
      ok = writeToFlash(path, data) || ok;
    }
    if (_sdReady) {
      ok = writeToSd(path, data) || ok;
    }
    return ok;
  }

  if (_sdReady && writeToSd(path, data)) {
    return true;
  }
  if (_flashReady && writeToFlash(path, data)) {
    return true;
  }
  return false;
}

bool StorageManager::writeTextFileSdOnly(const char* path, const String& data) {
  if (!_sdReady || !path) {
    return false;
  }
  ensureDirectory(path);
  return writeToSd(path, data);
}

bool StorageManager::appendTextFileSdOnly(const char* path, const String& data) {
  if (!_sdReady || !path) {
    return false;
  }
  ensureDirectory(path);
  String existing;
  if (readFromSd(path, existing)) {
    return writeToSd(path, existing + data);
  }
  return writeToSd(path, data);
}

bool StorageManager::appendTextFile(const char* path, const String& data) {
  String existing;
  if (readTextFile(path, existing)) {
    return writeTextFile(path, existing + data);
  }
  return writeTextFile(path, data);
}

bool StorageManager::writeBinaryFile(const char* path, const uint8_t* data, size_t len) {
  if (!path || !data || len == 0) {
    return false;
  }
  if (_sdReady) {
    File file = SD.open(normalizeSdPath(path).c_str(), FILE_WRITE);
    if (file) {
      const size_t n = file.write(data, len);
      file.close();
      if (n == len) {
        return true;
      }
    }
  }
  if (_flashReady) {
    File file = LittleFS.open(normalizeFlashPath(path).c_str(), "w");
    if (file) {
      const size_t n = file.write(data, len);
      file.close();
      return n == len;
    }
  }
  return false;
}

bool StorageManager::writeBinaryFileSdOnly(const char* path, const uint8_t* data, size_t len) {
  if (!_sdReady || !path || !data || len == 0) {
    return false;
  }
  ensureDirectory(path);
  File file = SD.open(normalizeSdPath(path).c_str(), FILE_WRITE);
  if (!file) {
    return false;
  }
  const size_t n = file.write(data, len);
  file.close();
  return n == len;
}

bool StorageManager::appendBinaryFile(const char* path, const uint8_t* data, size_t len) {
  if (!path || !data || len == 0) {
    return false;
  }

  ensureDirectory(path);

  if (isDeviceConfigPath(path) || isWifiDataPath(path)) {
    bool ok = false;
    if (_flashReady) {
      File file = LittleFS.open(normalizeFlashPath(path).c_str(), "a");
      if (file) {
        const size_t n = file.write(data, len);
        file.close();
        ok = (n == len) || ok;
      }
    }
    if (_sdReady) {
      File file = SD.open(normalizeSdPath(path).c_str(), FILE_APPEND);
      if (file) {
        const size_t n = file.write(data, len);
        file.close();
        ok = (n == len) || ok;
      }
    }
    return ok;
  }

  if (_sdReady) {
    File file = SD.open(normalizeSdPath(path).c_str(), FILE_APPEND);
    if (file) {
      const size_t n = file.write(data, len);
      file.close();
      if (n == len) {
        return true;
      }
    }
  }
  if (_flashReady) {
    File file = LittleFS.open(normalizeFlashPath(path).c_str(), "a");
    if (file) {
      const size_t n = file.write(data, len);
      file.close();
      return n == len;
    }
  }
  return false;
}

bool StorageManager::appendBinaryFileSdOnly(const char* path, const uint8_t* data, size_t len) {
  if (!_sdReady || !path || !data || len == 0) {
    return false;
  }
  ensureDirectory(path);
  File file = SD.open(normalizeSdPath(path).c_str(), FILE_APPEND);
  if (!file) {
    return false;
  }
  const size_t n = file.write(data, len);
  file.close();
  return n == len;
}

size_t StorageManager::fileSize(const char* path) const {
  if (!path) {
    return 0;
  }
  if (_sdReady) {
    File file = SD.open(normalizeSdPath(path).c_str(), FILE_READ);
    if (file) {
      const size_t n = file.size();
      file.close();
      return n;
    }
  }
  if (_flashReady) {
    File file = LittleFS.open(normalizeFlashPath(path).c_str(), "r");
    if (file) {
      const size_t n = file.size();
      file.close();
      return n;
    }
  }
  return 0;
}

bool StorageManager::readBinaryFile(const char* path, uint8_t* out, size_t maxLen, size_t& readLen) const {
  readLen = 0;
  if (!path || !out || maxLen == 0) {
    return false;
  }
  if (_sdReady) {
    File file = SD.open(normalizeSdPath(path).c_str(), FILE_READ);
    if (file) {
      readLen = file.read(out, maxLen);
      file.close();
      return readLen > 0;
    }
  }
  if (_flashReady) {
    File file = LittleFS.open(normalizeFlashPath(path).c_str(), "r");
    if (file) {
      readLen = file.read(out, maxLen);
      file.close();
      return readLen > 0;
    }
  }
  return false;
}

bool StorageManager::exists(const char* path) {
  if (_sdReady && SD.exists(normalizeSdPath(path))) {
    return true;
  }
  if (_flashReady && LittleFS.exists(normalizeFlashPath(path))) {
    return true;
  }
  return false;
}

bool StorageManager::readFromSd(const char* path, String& out) {
  File file = SD.open(normalizeSdPath(path).c_str(), FILE_READ);
  if (!file) {
    return false;
  }
  out = file.readString();
  file.close();
  return true;
}

bool StorageManager::readFromFlash(const char* path, String& out) {
  File file = LittleFS.open(normalizeFlashPath(path).c_str(), "r");
  if (!file) {
    return false;
  }
  out = file.readString();
  file.close();
  return true;
}

bool StorageManager::writeToSd(const char* path, const String& data) {
  File file = SD.open(normalizeSdPath(path).c_str(), FILE_WRITE);
  if (!file) {
    return false;
  }
  const size_t written = file.print(data);
  file.close();
  return written == data.length();
}

bool StorageManager::writeToFlash(const char* path, const String& data) {
  const String normalized = normalizeFlashPath(path);

  if (isDeviceConfigPath(path)) {
    const String tmpPath = normalized + ".tmp";
    LittleFS.remove(tmpPath.c_str());
    File file = LittleFS.open(tmpPath.c_str(), "w");
    if (!file) {
      return false;
    }
    const size_t written = file.print(data);
    file.close();
    if (written != data.length()) {
      LittleFS.remove(tmpPath.c_str());
      return false;
    }
    LittleFS.remove(normalized.c_str());
    if (!LittleFS.rename(tmpPath.c_str(), normalized.c_str())) {
      return false;
    }
    return true;
  }

  File file = LittleFS.open(normalized.c_str(), "w");
  if (!file) {
    return false;
  }
  const size_t written = file.print(data);
  file.close();
  return written == data.length();
}

bool StorageManager::canExportToSd(char* error, size_t errorLen) const {
  if (_sdReady) {
    return true;
  }
  const char* msg =
      "Insert an SD card (Full Mode). Export to SD is not available without a card.";
  if (error && errorLen > 0) {
    strncpy(error, msg, errorLen - 1);
    error[errorLen - 1] = '\0';
  }
  return false;
}

bool StorageManager::checkRequirement(SdRequirement requirement, String& message) const {
  switch (requirement) {
    case SdRequirement::NotNeeded:
      return true;
    case SdRequirement::Optional:
      if (!_sdReady && SD_SHOW_WARNINGS) {
        message = "SD not inserted. Basic mode works; insert SD for the best experience.";
      }
      return true;
    case SdRequirement::Required:
      if (_sdReady) {
        return true;
      }
      message = "SD card required for this feature. Insert SD and restart or run: sd status";
      return false;
  }
  return true;
}

const char* StorageManager::getStatusLine() const {
  return _statusLine;
}

const char* StorageManager::getTierLabel() const {
  return (_tier == StorageTier::SdFull) ? "Full Mode" : "Flash Mode";
}

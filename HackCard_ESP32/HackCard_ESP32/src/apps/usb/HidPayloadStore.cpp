#include "HidPayloadStore.h"

#include "../../core/StorageManager.h"

#include <LittleFS.h>
#include <SD.h>

HidPayloadStore HidPayloads;

namespace {

constexpr char kPayloadDir[] = "/payloads";

bool endsWithIgnoreCase(const char* text, const char* suffix) {
  if (!text || !suffix) {
    return false;
  }
  const size_t tLen = strlen(text);
  const size_t sLen = strlen(suffix);
  if (sLen > tLen) {
    return false;
  }
  for (size_t i = 0; i < sLen; i++) {
    char a = text[tLen - sLen + i];
    char b = suffix[i];
    if (a >= 'A' && a <= 'Z') {
      a = static_cast<char>(a - 'A' + 'a');
    }
    if (b >= 'A' && b <= 'Z') {
      b = static_cast<char>(b - 'A' + 'a');
    }
    if (a != b) {
      return false;
    }
  }
  return true;
}

}  // namespace

bool HidPayloadStore::hasAllowedExt(const char* filename) const {
  return endsWithIgnoreCase(filename, ".duck") || endsWithIgnoreCase(filename, ".txt") ||
         endsWithIgnoreCase(filename, ".ds");
}

bool HidPayloadStore::isSafeName(const char* name) const {
  if (!name || name[0] == '\0' || strlen(name) > 47) {
    return false;
  }
  if (strchr(name, '/') || strchr(name, '\\') || strstr(name, "..")) {
    return false;
  }
  return hasAllowedExt(name);
}

bool HidPayloadStore::buildPath(const char* name, char* out, size_t outLen, bool preferSd) const {
  if (!isSafeName(name) || !out || outLen < 16) {
    return false;
  }
  snprintf(out, outLen, "%s/%s", kPayloadDir, name);
  if (preferSd && Storage.isSdReady() && SD.exists(out)) {
    return true;
  }
  if (LittleFS.exists(out)) {
    return true;
  }
  if (Storage.isSdReady() && SD.exists(out)) {
    return true;
  }
  return false;
}

uint8_t HidPayloadStore::scanFolder(const char* folder, bool fromSd, HidPayloadEntry* out, uint8_t maxEntries,
                                    uint8_t startIndex) const {
  uint8_t count = startIndex;
#if defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
  File root = fromSd ? SD.open(folder) : LittleFS.open(folder);
  if (!root || !root.isDirectory()) {
    return count;
  }
  File file = root.openNextFile();
  while (file && count < maxEntries) {
    if (!file.isDirectory()) {
      const char* name = file.name();
      const char* base = strrchr(name, '/');
      base = base ? base + 1 : name;
      if (hasAllowedExt(base)) {
        bool duplicate = false;
        for (uint8_t i = 0; i < count; i++) {
          if (strcmp(out[i].name, base) == 0) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) {
          strlcpy(out[count].name, base, sizeof(out[count].name));
          snprintf(out[count].path, sizeof(out[count].path), "%s/%s", kPayloadDir, base);
          out[count].size = file.size();
          out[count].fromSd = fromSd;
          count++;
        }
      }
    }
    file = root.openNextFile();
  }
  root.close();
#else
  (void)folder;
  (void)fromSd;
  (void)out;
  (void)maxEntries;
  (void)startIndex;
#endif
  return count;
}

uint8_t HidPayloadStore::listEntries(HidPayloadEntry* out, uint8_t maxEntries) const {
  if (!out || maxEntries == 0) {
    return 0;
  }
  memset(out, 0, sizeof(HidPayloadEntry) * maxEntries);

  uint8_t count = 0;
  if (Storage.isSdReady()) {
    count = scanFolder(kPayloadDir, true, out, maxEntries, count);
  }
  count = scanFolder(kPayloadDir, false, out, maxEntries, count);
  return count;
}

bool HidPayloadStore::readPayload(const char* name, String& out) const {
  out = "";
  char path[96] = {0};
  if (!buildPath(name, path, sizeof(path), Storage.isSdReady())) {
    return false;
  }

  if (Storage.isSdReady() && SD.exists(path)) {
    File file = SD.open(path, FILE_READ);
    if (!file) {
      return false;
    }
    if (file.size() > kMaxScriptBytes) {
      file.close();
      return false;
    }
    while (file.available()) {
      out += static_cast<char>(file.read());
    }
    file.close();
    return true;
  }

  return Storage.readTextFile(path, out) && out.length() <= kMaxScriptBytes;
}

bool HidPayloadStore::payloadExists(const char* name) const {
  char path[96] = {0};
  return buildPath(name, path, sizeof(path), Storage.isSdReady());
}

namespace {

void setPayloadErr(char* errDetail, size_t errDetailLen, const char* msg) {
  if (errDetail && errDetailLen > 0 && msg) {
    strlcpy(errDetail, msg, errDetailLen);
  }
}

}  // namespace

bool HidPayloadStore::writePayload(const char* name, const String& content, char* errDetail, size_t errDetailLen) {
  if (!isSafeName(name)) {
    setPayloadErr(errDetail, errDetailLen,
                  "Invalid filename — use letters, numbers, dash, underscore, ending in .duck or .txt");
    return false;
  }
  if (content.length() == 0) {
    setPayloadErr(errDetail, errDetailLen, "Script body was empty on device — retry Save (firmware accepts raw text body)");
    return false;
  }
  if (content.length() > kMaxScriptBytes) {
    setPayloadErr(errDetail, errDetailLen, "Script too large (max 4 KB)");
    return false;
  }
  Storage.ensureFlashMounted();
  if (!Storage.isFlashReady() && !Storage.isSdReady()) {
    setPayloadErr(errDetail, errDetailLen,
                  "Storage not ready — in Arduino: Custom partition (partitions.csv), then Tools → ESP32 Sketch Data Upload, reflash");
    return false;
  }
  if (!Storage.ensureDirectory("/payloads/_.duck")) {
    setPayloadErr(errDetail, errDetailLen, "Could not create /payloads/ folder");
    return false;
  }
  char path[96] = {0};
  snprintf(path, sizeof(path), "%s/%s", kPayloadDir, name);
  if (!Storage.writeTextFile(path, content)) {
    const size_t freeB = Storage.freeFlashBytes();
    if (freeB < 512 && !Storage.isSdReady()) {
      setPayloadErr(errDetail, errDetailLen,
                    "Flash may be full — insert SD and use Save to SD, or free LittleFS space");
    } else if (freeB < 512) {
      setPayloadErr(errDetail, errDetailLen, "Write failed — flash low on space; try Save to SD");
    } else {
      setPayloadErr(errDetail, errDetailLen, "Write failed — check SD card or reboot and retry");
    }
    return false;
  }
  return true;
}

bool HidPayloadStore::writePayloadSdOnly(const char* name, const String& content) {
  if (!Storage.isSdReady()) {
    return false;
  }
  if (!isSafeName(name) || content.length() == 0 || content.length() > kMaxScriptBytes) {
    return false;
  }
  char path[96] = {0};
  snprintf(path, sizeof(path), "%s/%s", kPayloadDir, name);
  if (!Storage.ensureDirectory(kPayloadDir)) {
    return false;
  }
  return Storage.writeTextFileSdOnly(path, content);
}

bool HidPayloadStore::deletePayload(const char* name) {
  if (!isSafeName(name)) {
    return false;
  }
  char path[96] = {0};
  snprintf(path, sizeof(path), "%s/%s", kPayloadDir, name);
  if (Storage.isSdReady() && SD.exists(path)) {
    return SD.remove(path);
  }
  if (LittleFS.exists(path)) {
    return LittleFS.remove(path);
  }
  return false;
}

String HidPayloadStore::listToJson() const {
  HidPayloadEntry entries[kMaxEntries];
  const uint8_t count = listEntries(entries, kMaxEntries);
  String json = "{\"count\":" + String(count) + ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"flash_ready\":";
  json += Storage.isFlashReady() ? "true" : "false";
  json += ",\"flash_free_bytes\":";
  json += String(static_cast<unsigned>(Storage.freeFlashBytes()));
  json += ",\"storage\":\"";
  json += Storage.isSdReady() ? "sd" : "flash";
  json += "\",\"payloads\":[";
  for (uint8_t i = 0; i < count; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"name\":\"" + String(entries[i].name) + "\",\"size\":" + String(entries[i].size) + ",\"source\":\"";
    json += entries[i].fromSd ? "sd" : "flash";
    json += "\"}";
  }
  json += "]}";
  return json;
}

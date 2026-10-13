#pragma once

#include <Arduino.h>

struct HidPayloadEntry {
  char name[48] = {0};
  char path[80] = {0};
  uint32_t size = 0;
  bool fromSd = false;
};

class HidPayloadStore {
 public:
  static constexpr uint8_t kMaxEntries = 24;
  static constexpr size_t kMaxScriptBytes = 4096;

  uint8_t listEntries(HidPayloadEntry* out, uint8_t maxEntries) const;
  bool readPayload(const char* name, String& out) const;
  bool writePayload(const char* name, const String& content, char* errDetail = nullptr, size_t errDetailLen = 0);
  bool writePayloadSdOnly(const char* name, const String& content);
  bool deletePayload(const char* name);
  bool payloadExists(const char* name) const;
  String listToJson() const;

 private:
  bool isSafeName(const char* name) const;
  bool buildPath(const char* name, char* out, size_t outLen, bool preferSd) const;
  uint8_t scanFolder(const char* folder, bool fromSd, HidPayloadEntry* out, uint8_t maxEntries,
                     uint8_t startIndex) const;
  bool hasAllowedExt(const char* filename) const;
};

extern HidPayloadStore HidPayloads;

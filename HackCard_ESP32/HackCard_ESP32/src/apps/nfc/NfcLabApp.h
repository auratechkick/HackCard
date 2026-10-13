#pragma once

#include "../../platform/hal/HalNfc.h"

struct NfcCloneResult {
  bool success = false;
  char sourcePath[64] = {0};
  char destUid[64] = {0};
  uint8_t pagesWritten = 0;
  char error[64] = {0};
};

struct NfcKillResult {
  bool success = false;
  char uid[64] = {0};
  char error[64] = {0};
};

class NfcCloneApp {
public:
  bool ensureReady();
  NfcCloneResult restoreDump(const char* dumpPath, const char* labPin, uint8_t startPage = 4);
  NfcCloneResult restoreLast(const char* labPin);
  String toJson(const NfcCloneResult& result) const;
  String toText(const NfcCloneResult& result) const;
};

class NfcKillApp {
public:
  bool ensureReady();
  NfcKillResult killTag(const char* labPin);
  String toJson(const NfcKillResult& result) const;
  String toText(const NfcKillResult& result) const;
};

extern NfcCloneApp NfcClone;
extern NfcKillApp NfcKill;

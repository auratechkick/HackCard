#pragma once

#include "../../platform/hal/HalNfc.h"

struct NfcClassicResult {
  bool success = false;
  NfcClassicDump dump;
  String savedPath;
  char error[64] = {0};
};

struct NfcKeyScanResult {
  bool success = false;
  char keyHex[13] = {0};
  uint8_t sector = 0;
  char error[64] = {0};
};

class NfcClassicApp {
public:
  bool ensureReady();
  NfcClassicResult dumpWithKey(const char* keyHex, uint8_t maxSectors = 16);
  NfcClassicResult cloneLast(const char* keyHex, const char* labPin);
  NfcKeyScanResult scanKey(uint8_t sector = 0);
  String dumpToJson(const NfcClassicResult& result) const;
  String dumpToText(const NfcClassicResult& result) const;
  String keyScanToJson(const NfcKeyScanResult& result) const;
  String keyScanToText(const NfcKeyScanResult& result) const;
  String lastSavedPath() const { return _lastSavedPath; }

private:
  String formatClassicFile(const NfcClassicResult& result) const;
  bool saveClassicDump(NfcClassicResult& result);

  NfcClassicDump _lastDump;
  bool _hasLastDump = false;
  String _lastSavedPath;
};

extern NfcClassicApp NfcClassic;

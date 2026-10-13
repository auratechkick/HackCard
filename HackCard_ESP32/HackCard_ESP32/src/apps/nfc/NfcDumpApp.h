#pragma once

#include "../../platform/hal/HalNfc.h"

struct NfcDumpResult {
  bool success = false;
  NfcDumpData dump;
  String savedPath;
  String preview;
};

class NfcDumpApp {
public:
  bool ensureReady();
  NfcDumpResult dumpOnce(uint8_t maxPages = 45);
  String toJson(const NfcDumpResult& result) const;
  String toText(const NfcDumpResult& result) const;

private:
  String formatDumpFile(const NfcDumpResult& result) const;
  bool saveDump(NfcDumpResult& result);
};

extern NfcDumpApp NfcDump;

#pragma once

#include "../../platform/hal/HalNfc.h"

struct NfcReadResult {
  bool success = false;
  NfcTagInfo tag;
  String savedPath;
};

class NfcReadApp {
public:
  bool ensureReady();
  NfcReadResult readOnce(uint32_t timeoutMs = 1500);
  String toJson(const NfcReadResult& result) const;
  String toText(const NfcReadResult& result) const;

private:
  bool saveRead(const NfcTagInfo& tag, String& savedPath);
};

extern NfcReadApp NfcRead;

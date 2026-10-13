#pragma once

#include "../../platform/hal/HalNfc.h"

struct NfcInfoResult {
  bool success = false;
  NfcTagDetail tag;
};

class NfcInfoApp {
public:
  bool ensureReady();
  NfcInfoResult readInfo(uint32_t timeoutMs = 2000);
  String toJson(const NfcInfoResult& result) const;
  String toText(const NfcInfoResult& result) const;
};

extern NfcInfoApp NfcInfo;

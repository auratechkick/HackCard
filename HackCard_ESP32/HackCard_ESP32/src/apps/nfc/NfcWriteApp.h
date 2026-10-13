#pragma once

#include <Arduino.h>

#include "../../core/AppContext.h"

struct NfcWriteResult {
  bool success = false;
  char writeType[12] = {0};
  char summary[128] = {0};
  char error[64] = {0};
};

struct NfcEraseResult {
  bool success = false;
  char error[64] = {0};
};

class NfcWriteApp {
public:
  bool ensureReady();
  NfcWriteResult writeUri(const char* url);
  NfcWriteResult writeText(const char* text);
  NfcWriteResult writeVcard(const HackCardConfig& profile);
  NfcEraseResult eraseTag(const char* labPin);
  String writeJson(const NfcWriteResult& result) const;
  String eraseJson(const NfcEraseResult& result) const;

private:
  NfcWriteResult runWrite(bool (*writer)(const char*, char*, size_t), const char* payload, const char* type,
                          const char* summary);
  String buildNfcVcard(const HackCardConfig& profile) const;
};

extern NfcWriteApp NfcWrite;

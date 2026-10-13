#pragma once

#include "../../platform/hal/HalNfc.h"

#include <Arduino.h>

class NfcDumpParser {
public:
  static bool parseDumpFile(const String& text, NfcDumpData& out, char* errorOut = nullptr,
                            size_t errorOutSize = 0);
  static bool loadDumpPath(const char* path, NfcDumpData& out, char* errorOut = nullptr,
                           size_t errorOutSize = 0);
};

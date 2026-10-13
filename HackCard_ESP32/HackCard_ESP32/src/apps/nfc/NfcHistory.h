#pragma once

#include <Arduino.h>

class NfcHistory {
public:
  void append(const char* action, const char* uid, const char* detail);
  String readRecent(uint16_t maxLines = 20);
  String lastDumpPath() const;
  void setLastDumpPath(const char* path);

  void setHistorySdOnly(bool enabled) { _historySdOnly = enabled; }
  void setDumpsSdOnly(bool enabled) { _dumpsSdOnly = enabled; }
  bool historySdOnly() const { return _historySdOnly; }
  bool dumpsSdOnly() const { return _dumpsSdOnly; }

  bool exportHistoryToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const;
  bool exportLastDumpToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const;
  bool exportPathCopyToSd(const char* sourcePath, const char* prefix, char* outPath, size_t outPathLen,
                          char* error, size_t errorLen) const;

private:
  const char* historyPath() const;
  bool _historySdOnly = false;
  bool _dumpsSdOnly = false;
};

extern NfcHistory NfcHist;

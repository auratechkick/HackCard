#pragma once

#include <Arduino.h>

struct DiagResult {
  String name;
  bool pass;
  String detail;
};

struct DiagReport {
  bool allPass = false;
  DiagResult items[8];
  uint8_t count = 0;
};

class DiagnosticsApp {
public:
  DiagReport run(bool peripheralsReady);
  String toJson(const DiagReport& report) const;
  String toText(const DiagReport& report) const;

private:
  void addItem(DiagReport& report, const char* name, bool pass, const String& detail) const;
};

extern DiagnosticsApp Diagnostics;

#pragma once

#include <Arduino.h>

/** Structured firmware logs → USB Serial; activity rows → /logs/activity.csv. See docs/SERIAL_AND_LOGGING.md */
class Logger {
public:
  void begin(bool enabled);
  void info(const char* tag, const char* message);
  void warn(const char* tag, const char* message);
  void error(const char* tag, const char* message);
  void activity(const char* app, const char* action, const char* result);

private:
  void writeLine(const char* level, const char* tag, const char* message);
  bool _enabled = true;
};

extern Logger Log;

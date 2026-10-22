#pragma once

#include <Arduino.h>

// Runs a Ducky Script on the PC this board is plugged into.
// GUI is the USB logo key: Windows key, or Command on a Mac.
// Select the OS with Host before running a script that opens apps.

class DuckyRunner {
public:
  bool run(const char* script);
  const char* lastError() const { return _error; }
  uint16_t defaultDelay() const { return _defaultDelayMs; }

private:
  bool handleLine(const char* line);
  bool executeLine(const char* line, bool remember);
  bool runKeys(char* line);
  bool lookupKey(const char* token, uint8_t& keyOut) const;
  void fail(const char* message);

  char _lastLine[192] = {0};
  char _error[80] = {0};
  uint16_t _defaultDelayMs = 0;
};

extern DuckyRunner Ducky;

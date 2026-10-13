#pragma once

#include <Arduino.h>

// Ring buffer of firmware log lines for the web UI serial panel.
class SerialLogBuffer {
public:
  void begin();
  void append(const char* text, size_t len);
  void appendLine(const char* line);
  void appendf(const char* fmt, ...);

  // Returns new text since `cursor` (updated on return). Plain text.
  String readSince(uint32_t& cursor) const;
  uint32_t length() const { return _len; }
  void clear();

private:
  static constexpr size_t kCapacity = 8192;
  char _buf[kCapacity] = {};
  size_t _len = 0;
};

extern SerialLogBuffer SerialLog;

#pragma once

#include <Arduino.h>

class ActivityLog {
public:
  bool readTail(String& out, uint16_t maxLines = 40);
  bool clear();
  bool exists();
};

extern ActivityLog ActivityLogs;

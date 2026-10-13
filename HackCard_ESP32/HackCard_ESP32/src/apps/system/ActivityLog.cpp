#include "ActivityLog.h"

#include "../../core/StorageManager.h"

ActivityLog ActivityLogs;

namespace {

constexpr const char* kLogPath = "/logs/activity.csv";

}  // namespace

bool ActivityLog::exists() {
  return Storage.exists(kLogPath);
}

bool ActivityLog::readTail(String& out, uint16_t maxLines) {
  String raw;
  if (!Storage.readTextFile(kLogPath, raw) || raw.isEmpty()) {
    out = "No activity logs yet.\n";
    return false;
  }

  String lines[64];
  uint16_t count = 0;
  int lineStart = 0;

  for (int i = 0; i <= raw.length(); i++) {
    if (i == raw.length() || raw[i] == '\n') {
      if (i > lineStart && count < 64) {
        lines[count++] = raw.substring(lineStart, i);
      }
      lineStart = i + 1;
    }
  }

  out = "";
  const uint16_t start = (count > maxLines) ? (count - maxLines) : 0;
  for (uint16_t i = start; i < count; i++) {
    out += lines[i];
    out += "\n";
  }
  return true;
}

bool ActivityLog::clear() {
  return Storage.writeTextFile(kLogPath, "ms,app,action,result\n");
}

#include "Logger.h"

#include "SerialLogBuffer.h"
#include "StorageManager.h"

Logger Log;

void Logger::begin(bool enabled) {
  _enabled = enabled;
}

void Logger::writeLine(const char* level, const char* tag, const char* message) {
  if (!_enabled) {
    return;
  }

  SerialLog.appendf("[%lu][%s][%s] %s\n", millis(), level, tag, message);
  Serial.printf("[%lu][%s][%s] %s\n", millis(), level, tag, message);
}

void Logger::info(const char* tag, const char* message) {
  writeLine("INFO", tag, message);
}

void Logger::warn(const char* tag, const char* message) {
  writeLine("WARN", tag, message);
}

void Logger::error(const char* tag, const char* message) {
  writeLine("ERROR", tag, message);
}

void Logger::activity(const char* app, const char* action, const char* result) {
  if (!_enabled) {
    return;
  }

  SerialLog.appendf("[%lu][ACT][%s] %s -> %s\n", millis(), app, action, result);
  Serial.printf("[%lu][ACT][%s] %s -> %s\n", millis(), app, action, result);

  if (Storage.isFlashReady() || Storage.isSdReady()) {
    if (!Storage.exists("/logs/activity.csv")) {
      Storage.writeTextFile("/logs/activity.csv", "ms,app,action,result\n");
    }
    String line = String(millis()) + "," + app + "," + action + "," + result + "\n";
    Storage.appendTextFile("/logs/activity.csv", line);
  }
}

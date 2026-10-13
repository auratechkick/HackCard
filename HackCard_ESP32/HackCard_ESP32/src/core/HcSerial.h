#pragma once

#include <Arduino.h>

#include "SerialLogBuffer.h"

/**
 * HackCard serial output facade (115200 baud on USB CDC).
 * Mirrors text to USB Serial and SerialLogBuffer (web Terminal page).
 */
class HcSerialClass {
public:
  void print(const char* text);
  void print(char* text) { print(static_cast<const char*>(text)); }
  void print(const String& text);
  void println();
  void println(const char* text);
  void println(char* text) { println(static_cast<const char*>(text)); }
  void println(const String& text);
  template <typename T>
  void println(T value) {
    printValue(value);
    appendNewline();
  }
  void printf(const char* fmt, ...);

private:
  void appendNewline();
  void printValue(const char* text);
  void printValue(const String& text);
  template <typename T>
  void printValue(T value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(value));
    SerialLog.append(buffer, strlen(buffer));
    Serial.print(value);
  }
};

extern HcSerialClass HcSerial;

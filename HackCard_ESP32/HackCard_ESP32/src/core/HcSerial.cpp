#include "HcSerial.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

HcSerialClass HcSerial;

void HcSerialClass::print(const char* text) {
  printValue(text);
}

void HcSerialClass::print(const String& text) {
  printValue(text);
}

void HcSerialClass::println() {
  appendNewline();
}

void HcSerialClass::println(const char* text) {
  printValue(text);
  appendNewline();
}

void HcSerialClass::println(const String& text) {
  printValue(text);
  appendNewline();
}

void HcSerialClass::printf(const char* fmt, ...) {
  if (!fmt) {
    return;
  }
  char buffer[256];
  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  if (n > 0) {
    SerialLog.append(buffer, static_cast<size_t>(n));
    Serial.printf("%s", buffer);
  }
}

void HcSerialClass::appendNewline() {
  SerialLog.append("\n", 1);
  Serial.println();
}

void HcSerialClass::printValue(const char* text) {
  if (!text) {
    return;
  }
  SerialLog.append(text, strlen(text));
  Serial.print(text);
}

void HcSerialClass::printValue(const String& text) {
  SerialLog.append(text.c_str(), text.length());
  Serial.print(text);
}

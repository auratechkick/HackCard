#include "SerialLogBuffer.h"

#include <cstdarg>
#include <cstring>

SerialLogBuffer SerialLog;

void SerialLogBuffer::begin() {
  clear();
  appendLine("[log] Serial log buffer ready");
}

void SerialLogBuffer::append(const char* text, size_t len) {
  if (!text || len == 0) {
    return;
  }
  for (size_t i = 0; i < len; i++) {
    _buf[_len % kCapacity] = text[i];
    _len++;
  }
}

void SerialLogBuffer::appendLine(const char* line) {
  if (!line) {
    return;
  }
  append(line, strlen(line));
  append("\n", 1);
}

void SerialLogBuffer::appendf(const char* fmt, ...) {
  if (!fmt) {
    return;
  }
  char buffer[256];
  va_list args;
  va_start(args, fmt);
  const int n = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  if (n > 0) {
    append(buffer, static_cast<size_t>(n));
  }
}

String SerialLogBuffer::readSince(uint32_t& cursor) const {
  if (cursor >= _len) {
    cursor = _len;
    return String();
  }

  const size_t start = cursor % kCapacity;
  const size_t count = _len - cursor;
  String out;
  out.reserve(count + 1);

  if (start + count <= kCapacity) {
    out.concat(_buf + start, count);
  } else {
    const size_t first = kCapacity - start;
    out.concat(_buf + start, first);
    out.concat(_buf, count - first);
  }

  cursor = _len;
  return out;
}

void SerialLogBuffer::clear() {
  memset(_buf, 0, sizeof(_buf));
  _len = 0;
}

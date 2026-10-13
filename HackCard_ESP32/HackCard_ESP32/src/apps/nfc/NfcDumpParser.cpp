#include "NfcDumpParser.h"

#include "../../core/StorageManager.h"

#include <cstring>

namespace {

bool parsePageLine(const String& line, uint8_t page, uint8_t out[4]) {
  const int eq = line.indexOf('=');
  if (eq < 0) {
    return false;
  }

  String hexPart = line.substring(eq + 1);
  hexPart.trim();
  hexPart.replace(" ", "");

  if (hexPart.length() < 8) {
    return false;
  }

  for (uint8_t b = 0; b < 4; b++) {
    const String byteStr = hexPart.substring(b * 2, b * 2 + 2);
    out[b] = static_cast<uint8_t>(strtoul(byteStr.c_str(), nullptr, 16));
  }

  (void)page;
  return true;
}

}  // namespace

bool NfcDumpParser::parseDumpFile(const String& text, NfcDumpData& out, char* errorOut, size_t errorOutSize) {
  out = NfcDumpData();

  int lineStart = 0;
  for (int i = 0; i <= text.length(); i++) {
    if (i != text.length() && text[i] != '\n') {
      continue;
    }

    String line = text.substring(lineStart, i);
    line.trim();
    lineStart = i + 1;

    if (line.isEmpty() || line.startsWith("#")) {
      continue;
    }

    if (line.startsWith("uid=")) {
      strlcpy(out.tag.uidHex, line.substring(4).c_str(), sizeof(out.tag.uidHex));
      out.tag.detected = true;
      continue;
    }

    if (line.startsWith("type=")) {
      strlcpy(out.tag.tagType, line.substring(5).c_str(), sizeof(out.tag.tagType));
      continue;
    }

    if (line.startsWith("page")) {
      const int eqPos = line.indexOf('=');
      if (eqPos < 0) {
        continue;
      }
      const int pageNum = line.substring(4, eqPos).toInt();
      if (pageNum >= 0 && pageNum < 48) {
        if (parsePageLine(line, static_cast<uint8_t>(pageNum), out.pages[pageNum])) {
          const uint8_t count = static_cast<uint8_t>(pageNum) + 1;
          if (count > out.pageCount) {
            out.pageCount = count;
          }
        }
      }
    }
  }

  if (out.pageCount < 5) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, "Dump file has too few pages", errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    return false;
  }

  out.success = true;
  return true;
}

bool NfcDumpParser::loadDumpPath(const char* path, NfcDumpData& out, char* errorOut, size_t errorOutSize) {
  String text;
  if (!path || path[0] == '\0' || !Storage.readTextFile(path, text)) {
    if (errorOut && errorOutSize > 0) {
      strncpy(errorOut, "Could not read dump file", errorOutSize - 1);
      errorOut[errorOutSize - 1] = '\0';
    }
    return false;
  }

  return parseDumpFile(text, out, errorOut, errorOutSize);
}

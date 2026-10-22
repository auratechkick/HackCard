#include "DuckyRunner.h"

#include "HidHost.h"

#include "HalUsbHid.h"
#include <cstring>

DuckyRunner Ducky;

namespace {

void trimInPlace(char* text) {
  if (!text) {
    return;
  }
  char* start = text;
  while (*start == ' ' || *start == '\t' || *start == '\r') {
    start++;
  }
  if (start != text) {
    memmove(text, start, strlen(start) + 1);
  }
  size_t len = strlen(text);
  while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\t' || text[len - 1] == '\r')) {
    text[--len] = '\0';
  }
}

void upperCopy(char* dest, size_t destSize, const char* src) {
  size_t i = 0;
  if (!dest || destSize == 0) {
    return;
  }
  if (!src) {
    dest[0] = '\0';
    return;
  }
  while (src[i] && i + 1 < destSize) {
    char c = src[i];
    if (c >= 'a' && c <= 'z') {
      c = static_cast<char>(c - 'a' + 'A');
    }
    dest[i++] = c;
  }
  dest[i] = '\0';
}

bool startsWithWord(const char* line, const char* word) {
  const size_t n = strlen(word);
  if (strncmp(line, word, n) != 0) {
    return false;
  }
  return line[n] == '\0' || line[n] == ' ' || line[n] == '\t';
}

const char* skipWord(const char* line, size_t wordLen) {
  const char* rest = line + wordLen;
  if (*rest == ' ' || *rest == '\t') {
    rest++;
  }
  return rest;
}

}  // namespace

void DuckyRunner::fail(const char* message) {
  strncpy(_error, message ? message : "Ducky script failed", sizeof(_error) - 1);
  _error[sizeof(_error) - 1] = '\0';
}

bool DuckyRunner::lookupKey(const char* token, uint8_t& keyOut) const {
  char name[24];
  upperCopy(name, sizeof(name), token);
  if (name[0] == '\0') {
    return false;
  }

  if (strcmp(name, "ENTER") == 0 || strcmp(name, "RETURN") == 0) {
    keyOut = KEY_RETURN;
    return true;
  }
  if (strcmp(name, "TAB") == 0) {
    keyOut = KEY_TAB;
    return true;
  }
  if (strcmp(name, "ESC") == 0 || strcmp(name, "ESCAPE") == 0) {
    keyOut = KEY_ESC;
    return true;
  }
  if (strcmp(name, "SPACE") == 0) {
    keyOut = ' ';
    return true;
  }
  if (strcmp(name, "BACKSPACE") == 0) {
    keyOut = KEY_BACKSPACE;
    return true;
  }
  if (strcmp(name, "DELETE") == 0) {
    keyOut = KEY_DELETE;
    return true;
  }
  if (strcmp(name, "UP") == 0 || strcmp(name, "UPARROW") == 0) {
    keyOut = KEY_UP_ARROW;
    return true;
  }
  if (strcmp(name, "DOWN") == 0 || strcmp(name, "DOWNARROW") == 0) {
    keyOut = KEY_DOWN_ARROW;
    return true;
  }
  if (strcmp(name, "LEFT") == 0 || strcmp(name, "LEFTARROW") == 0) {
    keyOut = KEY_LEFT_ARROW;
    return true;
  }
  if (strcmp(name, "RIGHT") == 0 || strcmp(name, "RIGHTARROW") == 0) {
    keyOut = KEY_RIGHT_ARROW;
    return true;
  }
  if (strcmp(name, "HOME") == 0) {
    keyOut = KEY_HOME;
    return true;
  }
  if (strcmp(name, "END") == 0) {
    keyOut = KEY_END;
    return true;
  }
  if (strcmp(name, "PAGEUP") == 0) {
    keyOut = KEY_PAGE_UP;
    return true;
  }
  if (strcmp(name, "PAGEDOWN") == 0) {
    keyOut = KEY_PAGE_DOWN;
    return true;
  }
  if (strcmp(name, "INSERT") == 0) {
    keyOut = KEY_INSERT;
    return true;
  }
  if (strcmp(name, "CAPSLOCK") == 0) {
    keyOut = KEY_CAPS_LOCK;
    return true;
  }
  if (strcmp(name, "PRINTSCREEN") == 0) {
    keyOut = KEY_PRINT_SCREEN;
    return true;
  }
  if (strcmp(name, "MENU") == 0 || strcmp(name, "APP") == 0) {
    keyOut = KEY_MENU;
    return true;
  }
  if (strcmp(name, "GUI") == 0 || strcmp(name, "WINDOWS") == 0 || strcmp(name, "COMMAND") == 0) {
    keyOut = KEY_LEFT_GUI;
    return true;
  }
  if (strcmp(name, "ALT") == 0) {
    keyOut = KEY_LEFT_ALT;
    return true;
  }
  if (strcmp(name, "CTRL") == 0 || strcmp(name, "CONTROL") == 0) {
    keyOut = KEY_LEFT_CTRL;
    return true;
  }
  if (strcmp(name, "SHIFT") == 0) {
    keyOut = KEY_LEFT_SHIFT;
    return true;
  }

  if (name[0] == 'F' && name[1] != '\0') {
    bool digits = true;
    for (size_t i = 1; name[i] != '\0'; i++) {
      if (name[i] < '0' || name[i] > '9') {
        digits = false;
        break;
      }
    }
    if (digits) {
      const int fn = atoi(name + 1);
      if (fn >= 1 && fn <= 12) {
        static const uint8_t kFn[] = {KEY_F1, KEY_F2,  KEY_F3,  KEY_F4,  KEY_F5,  KEY_F6,
                                       KEY_F7, KEY_F8,  KEY_F9,  KEY_F10, KEY_F11, KEY_F12};
        keyOut = kFn[fn - 1];
        return true;
      }
    }
  }

  if (strlen(token) == 1) {
    keyOut = static_cast<uint8_t>(token[0]);
    return true;
  }
  return false;
}

bool DuckyRunner::runKeys(char* line) {
  uint8_t keys[6];
  uint8_t count = 0;
  char* cursor = line;
  while (*cursor && count < 6) {
    while (*cursor == ' ' || *cursor == '\t') {
      cursor++;
    }
    if (*cursor == '\0') {
      break;
    }
    char* token = cursor;
    while (*cursor && *cursor != ' ' && *cursor != '\t') {
      cursor++;
    }
    if (*cursor) {
      *cursor++ = '\0';
    }
    if (!lookupKey(token, keys[count])) {
      fail("Unknown key");
      Serial.printf("[ducky] Unknown key: %s\n", token);
      return false;
    }
    count++;
  }
  if (count == 0) {
    return true;
  }
  for (uint8_t i = 0; i < count; i++) {
    UsbHid.press(keys[i]);
    delay(15);
  }
  delay(20);
  UsbHid.releaseAll();
  return true;
}

bool DuckyRunner::executeLine(const char* source, bool remember) {
  char line[192];
  strncpy(line, source, sizeof(line) - 1);
  line[sizeof(line) - 1] = '\0';
  trimInPlace(line);
  if (line[0] == '\0' || line[0] == '#') {
    return true;
  }

  char command[192];
  upperCopy(command, sizeof(command), line);

  if (startsWithWord(command, "REM")) {
    Serial.printf("[ducky] %s\n", line);
    return true;
  }
  if (remember) {
    strncpy(_lastLine, line, sizeof(_lastLine) - 1);
    _lastLine[sizeof(_lastLine) - 1] = '\0';
  }

  Serial.printf("[ducky] > %s\n", line);

  bool ok = true;
  if (startsWithWord(command, "DEFAULT_DELAY") || startsWithWord(command, "DEFAULTDELAY")) {
    const char* word = startsWithWord(command, "DEFAULT_DELAY") ? "DEFAULT_DELAY" : "DEFAULTDELAY";
    _defaultDelayMs = static_cast<uint16_t>(atoi(skipWord(command, strlen(word))));
  } else if (startsWithWord(command, "DELAY")) {
    delay(static_cast<unsigned long>(atoi(skipWord(command, strlen("DELAY")))));
  } else if (startsWithWord(command, "STRINGLN")) {
    Host.typeText(skipWord(line, strlen("STRINGLN")));
    UsbHid.writeKey(KEY_RETURN);
  } else if (startsWithWord(command, "STRING")) {
    Host.typeText(skipWord(line, strlen("STRING")));
  } else if (startsWithWord(command, "REPEAT")) {
    fail("REPEAT cannot be repeated");
    return false;
  } else {
    ok = runKeys(line);
  }

  if (ok && _defaultDelayMs > 0) {
    delay(_defaultDelayMs);
  }
  return ok;
}

bool DuckyRunner::handleLine(const char* source) {
  char line[192];
  strncpy(line, source, sizeof(line) - 1);
  line[sizeof(line) - 1] = '\0';
  trimInPlace(line);
  if (line[0] == '\0') {
    return true;
  }

  char command[192];
  upperCopy(command, sizeof(command), line);
  if (startsWithWord(command, "REPEAT")) {
    const int times = atoi(skipWord(command, strlen("REPEAT")));
    if (_lastLine[0] == '\0') {
      fail("REPEAT without a previous command");
      return false;
    }
    if (times < 1) {
      fail("REPEAT needs a count");
      return false;
    }
    Serial.printf("[ducky] > REPEAT %d\n", times);
    for (int i = 0; i < times; i++) {
      if (!executeLine(_lastLine, false)) {
        return false;
      }
    }
    return true;
  }
  return executeLine(line, true);
}

bool DuckyRunner::run(const char* script) {
  _error[0] = '\0';
  _lastLine[0] = '\0';
  _defaultDelayMs = 0;
  if (!script) {
    fail("Empty script");
    return false;
  }

  const char* cursor = script;
  while (*cursor) {
    char line[192];
    size_t n = 0;
    while (*cursor && *cursor != '\n' && n + 1 < sizeof(line)) {
      line[n++] = *cursor++;
    }
    line[n] = '\0';
    if (*cursor == '\n') {
      cursor++;
    } else if (*cursor) {
      fail("Line too long");
      return false;
    }
    if (!handleLine(line)) {
      return false;
    }
  }
  UsbHid.releaseAll();
  return true;
}


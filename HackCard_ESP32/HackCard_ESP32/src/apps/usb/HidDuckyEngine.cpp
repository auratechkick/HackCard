#include "HidDuckyEngine.h"

#include "config/app_registry.h"
#include "../../platform/hal/HalUsbHid.h"

#include <cstring>
#include <cstdlib>

#if APP_USB_HID
#include "USBHIDKeyboard.h"
#endif

namespace {

void trimInPlace(char* text) {
  if (!text) {
    return;
  }
  char* start = text;
  while (*start == ' ' || *start == '\t') {
    start++;
  }
  if (start != text) {
    memmove(text, start, strlen(start) + 1);
  }
  size_t len = strlen(text);
  while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\t' || text[len - 1] == '\r' ||
                     text[len - 1] == '\n')) {
    text[--len] = '\0';
  }
}

void toUpperInPlace(char* token) {
  if (!token) {
    return;
  }
  for (char* p = token; *p; ++p) {
    if (*p >= 'a' && *p <= 'z') {
      *p = static_cast<char>(*p - 'a' + 'A');
    }
  }
}

uint8_t modifierFromToken(const char* token) {
  char upper[16] = {0};
  strlcpy(upper, token ? token : "", sizeof(upper));
  toUpperInPlace(upper);
  if (strcmp(upper, "GUI") == 0 || strcmp(upper, "WINDOWS") == 0 || strcmp(upper, "COMMAND") == 0 ||
      strcmp(upper, "CMD") == 0) {
    return KEY_LEFT_GUI;
  }
  if (strcmp(upper, "ALT") == 0) {
    return KEY_LEFT_ALT;
  }
  if (strcmp(upper, "CTRL") == 0 || strcmp(upper, "CONTROL") == 0) {
    return KEY_LEFT_CTRL;
  }
  if (strcmp(upper, "SHIFT") == 0) {
    return KEY_LEFT_SHIFT;
  }
  return 0;
}

}  // namespace

bool HidDuckyEngine::resolveKeyToken(const char* token, uint8_t& outKey) const {
  if (!token || token[0] == '\0') {
    return false;
  }

  char upper[24] = {0};
  strlcpy(upper, token, sizeof(upper));
  toUpperInPlace(upper);

  if (strcmp(upper, "ENTER") == 0 || strcmp(upper, "RETURN") == 0) {
    outKey = KEY_RETURN;
    return true;
  }
  if (strcmp(upper, "TAB") == 0) {
    outKey = KEY_TAB;
    return true;
  }
  if (strcmp(upper, "ESC") == 0 || strcmp(upper, "ESCAPE") == 0) {
    outKey = KEY_ESC;
    return true;
  }
  if (strcmp(upper, "SPACE") == 0) {
    outKey = ' ';
    return true;
  }
  if (strcmp(upper, "DELETE") == 0 || strcmp(upper, "DEL") == 0) {
    outKey = KEY_DELETE;
    return true;
  }
  if (strcmp(upper, "BACKSPACE") == 0) {
    outKey = KEY_BACKSPACE;
    return true;
  }
  if (strcmp(upper, "UP") == 0 || strcmp(upper, "UPARROW") == 0) {
    outKey = KEY_UP_ARROW;
    return true;
  }
  if (strcmp(upper, "DOWN") == 0 || strcmp(upper, "DOWNARROW") == 0) {
    outKey = KEY_DOWN_ARROW;
    return true;
  }
  if (strcmp(upper, "LEFT") == 0 || strcmp(upper, "LEFTARROW") == 0) {
    outKey = KEY_LEFT_ARROW;
    return true;
  }
  if (strcmp(upper, "RIGHT") == 0 || strcmp(upper, "RIGHTARROW") == 0) {
    outKey = KEY_RIGHT_ARROW;
    return true;
  }
  if (strcmp(upper, "PRINTSCREEN") == 0 || strcmp(upper, "PRINT") == 0) {
    outKey = KEY_PRINT_SCREEN;
    return true;
  }
  if (upper[0] == 'F') {
    const int fn = atoi(upper + 1);
    if (fn >= 1 && fn <= 12) {
      outKey = static_cast<uint8_t>(KEY_F1 + fn - 1);
      return true;
    }
  }
  if (strlen(token) == 1) {
    outKey = static_cast<uint8_t>(token[0]);
    return true;
  }
  return false;
}

bool HidDuckyEngine::pressTokenKey(const char* token) {
#if APP_USB_HID
  uint8_t key = 0;
  if (resolveKeyToken(token, key)) {
    return UsbHid.pressChord(0, 0, key);
  }
  return UsbHid.pressNamedKey(token);
#else
  (void)token;
  return false;
#endif
}

bool HidDuckyEngine::pressModifierChord(uint8_t mod1, uint8_t mod2, uint8_t mod3, uint8_t key) {
#if APP_USB_HID
  (void)mod3;
  return UsbHid.pressChord(mod1, mod2, key);
#else
  (void)mod1;
  (void)mod2;
  (void)mod3;
  (void)key;
  return false;
#endif
}

bool HidDuckyEngine::runModifierLine(const char* cmd, const char* args, HidTargetOs os) {
#if APP_USB_HID
  uint8_t mods[2] = {modifierFromToken(cmd), 0};
  uint8_t modCount = mods[0] != 0 ? 1 : 0;

  char buffer[96] = {0};
  strlcpy(buffer, args ? args : "", sizeof(buffer));

  char* tokens[6] = {0};
  uint8_t tokenCount = 0;
  char* save = nullptr;
  for (char* tok = strtok_r(buffer, " ", &save); tok && tokenCount < 6; tok = strtok_r(nullptr, " ", &save)) {
    tokens[tokenCount++] = tok;
  }

  uint8_t key = 0;
  for (uint8_t i = 0; i < tokenCount; i++) {
    const uint8_t mod = modifierFromToken(tokens[i]);
    if (mod != 0) {
      if (modCount < 2) {
        mods[modCount++] = mod;
      }
      continue;
    }
    if (!resolveKeyToken(tokens[i], key)) {
      return false;
    }
    break;
  }

  if (key == 0 && args && args[0] != '\0' && tokenCount == 0) {
    uint8_t argKey = 0;
    if (resolveKeyToken(args, argKey)) {
      return pressModifierChord(mods[0], modCount > 1 ? mods[1] : 0, 0, argKey);
    }
    if (mods[0] == KEY_LEFT_GUI) {
      return UsbHid.runLauncherCommand(os, args);
    }
    return pressTokenKey(args);
  }

  if (key == 0 && mods[0] == KEY_LEFT_GUI && modCount == 1) {
    if (os == HidTargetOs::MacOS) {
      return UsbHid.pressChord(KEY_LEFT_GUI, 0, KEY_SPACE);
    }
    return UsbHid.pressChord(KEY_LEFT_GUI, 0, 'r');
  }

  return pressModifierChord(mods[0], modCount > 1 ? mods[1] : 0, 0, key);
#else
  (void)cmd;
  (void)args;
  (void)os;
  return false;
#endif
}

bool HidDuckyEngine::runActionLine(const char* actionName, HidTargetOs os) {
#if APP_USB_HID
  const HidAction action = hidActionFromString(actionName);
  switch (action) {
    case HidAction::Notepad:
      return UsbHid.openNotepad(os);
    case HidAction::Calculator:
      return UsbHid.openCalculator(os);
    case HidAction::Terminal:
      return UsbHid.openTerminal(os);
    case HidAction::TerminalInfo:
      return UsbHid.runTerminalInfo(os);
    case HidAction::LockScreen:
      return UsbHid.lockScreen(os);
    case HidAction::Screenshot:
      return UsbHid.takeScreenshot(os);
    case HidAction::AppSwitcher:
      return UsbHid.appSwitcher(os);
    case HidAction::CloseWindow:
      return UsbHid.closeWindow(os);
    case HidAction::VolumeMute:
      return UsbHid.volumeMute();
    case HidAction::VolumeUp:
      return UsbHid.volumeUp();
    case HidAction::VolumeDown:
      return UsbHid.volumeDown();
    case HidAction::MediaPlayPause:
      return UsbHid.mediaPlayPause();
    case HidAction::MediaNext:
      return UsbHid.mediaNext();
    case HidAction::MediaPrevious:
      return UsbHid.mediaPrevious();
    default:
      return UsbHid.openNotepad(os);
  }
#else
  (void)actionName;
  (void)os;
  return false;
#endif
}

bool HidDuckyEngine::executeLine(const char* line, HidTargetOs& os, uint8_t repeatNext) {
#if APP_USB_HID
  (void)repeatNext;
  if (!line || line[0] == '\0') {
    return true;
  }

  char work[256] = {0};
  strlcpy(work, line, sizeof(work));
  trimInPlace(work);
  if (work[0] == '\0' || work[0] == '#') {
    return true;
  }

  char* cmd = work;
  char* args = strchr(work, ' ');
  if (args) {
    *args = '\0';
    args++;
    trimInPlace(args);
  }

  char cmdUpper[32] = {0};
  strlcpy(cmdUpper, cmd, sizeof(cmdUpper));
  toUpperInPlace(cmdUpper);

  if (strcmp(cmdUpper, "REM") == 0) {
    return true;
  }
  if (strcmp(cmdUpper, "OS") == 0 && args && args[0] != '\0') {
    os = hidOsFromString(args);
    return true;
  }
  if (strcmp(cmdUpper, "DEFAULT_DELAY") == 0 || strcmp(cmdUpper, "DEFAULTDELAY") == 0) {
    _defaultDelayMs = static_cast<uint16_t>(atoi(args));
    return true;
  }
  if (strcmp(cmdUpper, "DELAY") == 0) {
    const uint32_t ms = args ? static_cast<uint32_t>(atoi(args)) : 0;
    delay(ms > 0 ? ms : _defaultDelayMs);
    return true;
  }
  if (strcmp(cmdUpper, "STRING") == 0) {
    return args && UsbHid.typeTextPayload(args);
  }
  if (strcmp(cmdUpper, "ENTER") == 0) {
    return UsbHid.pressChord(0, 0, KEY_RETURN);
  }
  if (strcmp(cmdUpper, "TAB") == 0) {
    return UsbHid.pressChord(0, 0, KEY_TAB);
  }
  if (strcmp(cmdUpper, "ESC") == 0 || strcmp(cmdUpper, "ESCAPE") == 0) {
    return UsbHid.pressChord(0, 0, KEY_ESC);
  }
  if (strcmp(cmdUpper, "SPACE") == 0) {
    return UsbHid.typeTextPayload(" ");
  }
  if (strcmp(cmdUpper, "ACTION") == 0) {
    return args && runActionLine(args, os);
  }
  if (strcmp(cmdUpper, "MOUSEMOVE") == 0 || strcmp(cmdUpper, "MOUSE_MOVE") == 0) {
    if (!args) {
      return false;
    }
    const char* comma = strchr(args, ',');
    if (!comma) {
      return false;
    }
    const int dx = atoi(args);
    const int dy = atoi(comma + 1);
    return UsbHid.mouseMove(static_cast<int8_t>(dx), static_cast<int8_t>(dy));
  }
  if (strcmp(cmdUpper, "MOUSECLICK") == 0 || strcmp(cmdUpper, "MOUSE_CLICK") == 0) {
    return UsbHid.mouseClick(args && args[0] ? args : "left");
  }
  if (modifierFromToken(cmdUpper) != 0) {
    return runModifierLine(cmdUpper, args, os);
  }

  return pressTokenKey(cmdUpper);
#else
  (void)line;
  (void)os;
  (void)repeatNext;
  return false;
#endif
}

HidDuckyResult HidDuckyEngine::runScript(const char* script, HidTargetOs os) {
  HidDuckyResult result;
  _defaultDelayMs = 0;
  _linesExecuted = 0;

#if !APP_USB_HID
  strncpy(result.error, "USB HID disabled", sizeof(result.error) - 1);
  return result;
#endif

  if (!script || script[0] == '\0') {
    strncpy(result.error, "Empty script", sizeof(result.error) - 1);
    return result;
  }
  if (!UsbHid.isReady()) {
    strncpy(result.error, UsbHid.lastError(), sizeof(result.error) - 1);
    return result;
  }
  if (!UsbHid.waitForHost(5000)) {
    strncpy(result.error, "USB host not connected", sizeof(result.error) - 1);
    return result;
  }

  char* copy = static_cast<char*>(malloc(strlen(script) + 1));
  if (!copy) {
    strncpy(result.error, "Out of memory", sizeof(result.error) - 1);
    return result;
  }
  strcpy(copy, script);

  char* savePtr = nullptr;
  char* line = strtok_r(copy, "\n", &savePtr);
  uint8_t pendingRepeat = 1;

  while (line) {
    char raw[256] = {0};
    strlcpy(raw, line, sizeof(raw));
    trimInPlace(line);

    if (line[0] != '\0' && line[0] != '#') {
      char cmd[32] = {0};
      strlcpy(cmd, line, sizeof(cmd));
      char* sp = strchr(cmd, ' ');
      if (sp) {
        *sp = '\0';
      }
      toUpperInPlace(cmd);

      if (strcmp(cmd, "REPEAT") == 0) {
        char* repArg = strchr(line, ' ');
        pendingRepeat = static_cast<uint8_t>(repArg ? atoi(repArg + 1) : 1);
        if (pendingRepeat == 0) {
          pendingRepeat = 1;
        }
        line = strtok_r(nullptr, "\n", &savePtr);
        continue;
      }

      const uint8_t times = pendingRepeat;
      pendingRepeat = 1;
      for (uint8_t i = 0; i < times; i++) {
        if (!executeLine(line, os, pendingRepeat)) {
          snprintf(result.error, sizeof(result.error), "Failed at line %u: %.48s", _linesExecuted + 1, raw);
          free(copy);
          return result;
        }
        _linesExecuted++;
        if (_defaultDelayMs > 0) {
          delay(_defaultDelayMs);
        }
      }
    }

    line = strtok_r(nullptr, "\n", &savePtr);
  }

  free(copy);
  result.success = true;
  result.linesExecuted = _linesExecuted;
  snprintf(result.error, sizeof(result.error), "OK — %u lines executed", _linesExecuted);
  return result;
}

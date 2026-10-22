#include "HidHost.h"

#include <Keyboard.h>
#include <cstring>

HidHost Host;

namespace {

bool equalsIgnoreCase(const char* a, const char* b) {
  if (!a || !b) {
    return false;
  }
  while (*a && *b) {
    char ca = *a++;
    char cb = *b++;
    if (ca >= 'A' && ca <= 'Z') {
      ca = static_cast<char>(ca - 'A' + 'a');
    }
    if (cb >= 'A' && cb <= 'Z') {
      cb = static_cast<char>(cb - 'A' + 'a');
    }
    if (ca != cb) {
      return false;
    }
  }
  return *a == '\0' && *b == '\0';
}

}  // namespace

void HidHost::setOs(HackOs os) {
  _os = os;
}

const char* HidHost::osName() const {
  switch (_os) {
    case HackOs::MacOS:
      return "macOS";
    case HackOs::Linux:
      return "Linux";
    default:
      return "Windows";
  }
}

bool HidHost::setOsFromName(const char* name) {
  if (!name) {
    return false;
  }
  if (equalsIgnoreCase(name, "win") || equalsIgnoreCase(name, "windows")) {
    _os = HackOs::Windows;
    return true;
  }
  if (equalsIgnoreCase(name, "mac") || equalsIgnoreCase(name, "macos") || equalsIgnoreCase(name, "osx")) {
    _os = HackOs::MacOS;
    return true;
  }
  if (equalsIgnoreCase(name, "linux")) {
    _os = HackOs::Linux;
    return true;
  }
  return false;
}

void HidHost::cycleOs() {
  switch (_os) {
    case HackOs::Windows:
      _os = HackOs::MacOS;
      break;
    case HackOs::MacOS:
      _os = HackOs::Linux;
      break;
    default:
      _os = HackOs::Windows;
      break;
  }
}

void HidHost::countdown(uint8_t seconds, const char* action) {
  Serial.printf("%s (%s)\n", action ? action : "HID", osName());
  for (uint8_t i = seconds; i > 0; i--) {
    Serial.printf("  %u...\n", i);
    delay(1000);
  }
}

uint8_t HidHost::commandModifier() const {
  return _os == HackOs::MacOS ? KEY_LEFT_GUI : KEY_LEFT_CTRL;
}

void HidHost::chord(uint8_t modifier, uint8_t key) {
  Keyboard.press(modifier);
  delay(20);
  Keyboard.press(key);
  delay(30);
  Keyboard.releaseAll();
}

void HidHost::chord3(uint8_t first, uint8_t second, uint8_t key) {
  Keyboard.press(first);
  delay(15);
  Keyboard.press(second);
  delay(15);
  Keyboard.press(key);
  delay(30);
  Keyboard.releaseAll();
}

void HidHost::typeText(const char* text) {
  if (!text) {
    return;
  }
  while (*text) {
    Keyboard.write(static_cast<uint8_t>(*text++));
    delay(12);
  }
}

void HidHost::typeLine(const char* text) {
  typeText(text);
  Keyboard.write(KEY_RETURN);
  delay(40);
}

void HidHost::openLauncher() {
  if (_os == HackOs::Windows) {
    chord(KEY_LEFT_GUI, 'r');
  } else if (_os == HackOs::MacOS) {
    chord(KEY_LEFT_GUI, ' ');
  } else {
    chord(KEY_LEFT_ALT, KEY_F2);
  }
  delay(700);
}

void HidHost::runCommand(const char* command) {
  if (!command || command[0] == '\0') {
    return;
  }
  openLauncher();
  typeText(command);
  delay(80);
  Keyboard.write(KEY_RETURN);
  delay(400);
}

void HidHost::openUrl(const char* url) {
  runCommand(url);
}

void HidHost::openNotes() {
  switch (_os) {
    case HackOs::MacOS:
      runCommand("TextEdit");
      break;
    case HackOs::Linux:
      runCommand("gedit");
      break;
    default:
      runCommand("notepad");
      break;
  }
  delay(900);
}

void HidHost::openCalculator() {
  switch (_os) {
    case HackOs::MacOS:
      runCommand("Calculator");
      break;
    case HackOs::Linux:
      runCommand("gnome-calculator");
      break;
    default:
      runCommand("calc");
      break;
  }
}

void HidHost::openTerminal() {
  switch (_os) {
    case HackOs::MacOS:
      runCommand("Terminal");
      break;
    case HackOs::Linux:
      runCommand("gnome-terminal");
      break;
    default:
      runCommand("cmd");
      break;
  }
  delay(800);
}

void HidHost::lockScreen() {
  if (_os == HackOs::MacOS) {
    chord3(KEY_LEFT_CTRL, KEY_LEFT_GUI, 'q');
    return;
  }
  // Windows Win+L. Linux GNOME Super+L. Some desktops use Ctrl+Alt+L instead.
  chord(KEY_LEFT_GUI, 'l');
}

bool HidHost::unlockScreen(const char* password) {
  if (!password || password[0] == '\0' || strcmp(password, "change-me") == 0) {
    Serial.println("[hid] Edit kPassword in the sketch before unlock");
    return false;
  }
  // Wake the lock screen so the password box is showing, then type.
  Keyboard.write(' ');
  delay(_os == HackOs::MacOS ? 800 : 500);
  typeText(password);
  delay(80);
  Keyboard.write(KEY_RETURN);
  return true;
}

void HidHost::screenshot() {
  switch (_os) {
    case HackOs::MacOS:
      chord3(KEY_LEFT_GUI, KEY_LEFT_SHIFT, '3');
      break;
    case HackOs::Linux:
      Keyboard.write(KEY_PRINT_SCREEN);
      delay(30);
      break;
    default:
      chord3(KEY_LEFT_GUI, KEY_LEFT_SHIFT, 's');
      break;
  }
}

void HidHost::appSwitcher() {
  const uint8_t mod = _os == HackOs::MacOS ? KEY_LEFT_GUI : KEY_LEFT_ALT;
  Keyboard.press(mod);
  delay(30);
  Keyboard.press(KEY_TAB);
  delay(40);
  Keyboard.release(KEY_TAB);
  delay(250);
  Keyboard.releaseAll();
}

void HidHost::closeWindow() {
  if (_os == HackOs::MacOS) {
    chord(KEY_LEFT_GUI, 'w');
    return;
  }
  chord(KEY_LEFT_ALT, KEY_F4);
}

void HidHost::selectAll() {
  chord(commandModifier(), 'a');
}

void HidHost::copyClipboard() {
  chord(commandModifier(), 'c');
}

void HidHost::pasteClipboard() {
  chord(commandModifier(), 'v');
}

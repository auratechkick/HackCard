#include "HalUsbHid.h"

#include "../../core/HcSerial.h"

#include "config/app_registry.h"

#include <cstring>

#if APP_USB_HID
#include "USB.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDKeyboard.h"
#include "USBHIDMouse.h"

extern "C" bool tud_mounted(void);
#endif

HalUsbHid UsbHid;

#if APP_USB_HID && defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0

namespace {

USBHIDKeyboard sKeyboard;
USBHIDMouse sMouse;
USBHIDConsumerControl sConsumer;

__attribute__((constructor(101))) void hackcardUsbHidEarlyBegin() {
  sKeyboard.begin();
  sKeyboard.setShiftKeyReports(true);
  sMouse.begin();
  sConsumer.begin();
}

bool lookupNamedKey(const char* keyName, uint8_t* outKey) {
  if (!keyName || keyName[0] == '\0' || !outKey) {
    return false;
  }

  struct KeyMap {
    const char* name;
    uint8_t code;
  };

  static const KeyMap keys[] = {
      {"tab", KEY_TAB},
      {"enter", KEY_RETURN},
      {"return", KEY_RETURN},
      {"esc", KEY_ESC},
      {"escape", KEY_ESC},
      {"backspace", KEY_BACKSPACE},
      {"delete", KEY_DELETE},
      {"up", KEY_UP_ARROW},
      {"down", KEY_DOWN_ARROW},
      {"left", KEY_LEFT_ARROW},
      {"right", KEY_RIGHT_ARROW},
      {"space", ' '},
      {"print", KEY_PRINT_SCREEN},
      {"printscreen", KEY_PRINT_SCREEN},
      {"win", KEY_LEFT_GUI},
      {"cmd", KEY_LEFT_GUI},
      {"ctrl", KEY_LEFT_CTRL},
      {"alt", KEY_LEFT_ALT},
      {"shift", KEY_LEFT_SHIFT},
  };

  for (const KeyMap& entry : keys) {
    if (strcmp(keyName, entry.name) == 0) {
      *outKey = entry.code;
      return true;
    }
  }
  return false;
}

}  // namespace

#endif

bool HalUsbHid::begin() {
#if APP_USB_HID
#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE != 0
  strncpy(_lastError, "Set USB Mode to USB-OTG (TinyUSB)", sizeof(_lastError) - 1);
  _ready = false;
  return false;
#else
  USB.manufacturerName("HackCard");
  USB.productName("HackCard HID");
  USB.serialNumber("HC-HID-1");

  sKeyboard.begin();
  sKeyboard.setShiftKeyReports(true);
  sMouse.begin();
  sConsumer.begin();

  if (!static_cast<bool>(USB)) {
    USB.begin();
  }

  _ready = true;
  waitForHost(2000);
  return true;
#endif
#else
  strncpy(_lastError, "USB HID disabled in app_registry.h", sizeof(_lastError) - 1);
  _ready = false;
  return false;
#endif
}

bool HalUsbHid::isHostConnected() const {
#if APP_USB_HID && defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0
  return _ready && tud_mounted();
#else
  return false;
#endif
}

bool HalUsbHid::waitForHost(uint32_t timeoutMs) const {
#if APP_USB_HID && defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0
  const uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (tud_mounted()) {
      return true;
    }
    delay(10);
  }
  return isHostConnected();
#else
  (void)timeoutMs;
  return false;
#endif
}

#if APP_USB_HID && defined(ARDUINO_USB_MODE) && ARDUINO_USB_MODE == 0

bool HalUsbHid::ensureHostReady() {
  _lastError[0] = '\0';
  if (!_ready) {
    strncpy(_lastError, "HID not ready", sizeof(_lastError) - 1);
    return false;
  }
  if (!waitForHost(3000)) {
    strncpy(_lastError, "USB host not ready", sizeof(_lastError) - 1);
    return false;
  }
  return true;
}

bool HalUsbHid::pressKey(uint8_t key) {
  if (!tud_mounted()) {
    strncpy(_lastError, "USB host not connected", sizeof(_lastError) - 1);
    return false;
  }
  if (sKeyboard.press(key) == 0) {
    strncpy(_lastError, "HID press failed", sizeof(_lastError) - 1);
    return false;
  }
  delay(40);
  return true;
}

bool HalUsbHid::pressCombo(uint8_t mod1, uint8_t mod2, uint8_t key) {
  if (!pressKey(mod1)) {
    return false;
  }
  if (mod2 != 0 && !pressKey(mod2)) {
    return false;
  }
  if (key != 0 && !pressKey(key)) {
    return false;
  }
  delay(120);
  return releaseAllKeys();
}

bool HalUsbHid::releaseAllKeys() {
  sKeyboard.releaseAll();
  delay(40);
  return true;
}

bool HalUsbHid::consumerPress(uint16_t code) {
  if (!ensureHostReady()) {
    return false;
  }
  sConsumer.press(code);
  delay(60);
  sConsumer.release();
  delay(40);
  return true;
}

bool HalUsbHid::openLauncher(HidTargetOs os) {
  switch (os) {
    case HidTargetOs::MacOS:
      HcSerial.println("[hid] Cmd+Space");
      return pressCombo(KEY_LEFT_GUI, 0, KEY_SPACE);
    case HidTargetOs::Linux:
      HcSerial.println("[hid] Alt+F2");
      return pressCombo(KEY_LEFT_ALT, 0, KEY_F2);
    case HidTargetOs::Windows:
    default:
      HcSerial.println("[hid] Win+R");
      return pressCombo(KEY_LEFT_GUI, 0, 'r');
  }
}

bool HalUsbHid::typeText(const char* text) {
  if (!text || text[0] == '\0') {
    return false;
  }

  if (!tud_mounted()) {
    strncpy(_lastError, "HID lost before typing", sizeof(_lastError) - 1);
    return false;
  }

  for (const char* p = text; *p != '\0'; p++) {
    if (sKeyboard.write(static_cast<uint8_t>(*p)) == 0) {
      strncpy(_lastError, "HID write failed", sizeof(_lastError) - 1);
      return false;
    }
    delay(55);
  }
  delay(80);
  return true;
}

bool HalUsbHid::sendEnter() {
  if (!pressKey(KEY_RETURN)) {
    return false;
  }
  return releaseAllKeys();
}

bool HalUsbHid::pressChord(uint8_t mod1, uint8_t mod2, uint8_t key) {
  if (!ensureHostReady()) {
    return false;
  }
  if (mod1 != 0 && !pressKey(mod1)) {
    return false;
  }
  if (mod2 != 0 && !pressKey(mod2)) {
    return false;
  }
  if (key != 0 && !pressKey(key)) {
    return false;
  }
  delay(120);
  return releaseAllKeys();
}

bool HalUsbHid::runLauncherCommand(HidTargetOs os, const char* command) {
  _lastError[0] = '\0';
  if (!_ready || !command || command[0] == '\0') {
    strncpy(_lastError, "HID not ready", sizeof(_lastError) - 1);
    return false;
  }
  if (!ensureHostReady()) {
    return false;
  }
  if (!openLauncher(os)) {
    return false;
  }

  delay(os == HidTargetOs::MacOS ? 700 : 900);

  if (!waitForHost(1000)) {
    strncpy(_lastError, "USB disconnected during payload", sizeof(_lastError) - 1);
    return false;
  }
  if (!typeText(command)) {
    return false;
  }
  delay(150);
  return sendEnter();
}

bool HalUsbHid::openNotepad(HidTargetOs os) {
  switch (os) {
    case HidTargetOs::MacOS:
      return runLauncherCommand(os, "TextEdit");
    case HidTargetOs::Linux:
      return runLauncherCommand(os, "gedit");
    case HidTargetOs::Windows:
    default:
      return runLauncherCommand(os, "notepad");
  }
}

bool HalUsbHid::openCalculator(HidTargetOs os) {
  switch (os) {
    case HidTargetOs::MacOS:
      return runLauncherCommand(os, "Calculator");
    case HidTargetOs::Linux:
      return runLauncherCommand(os, "gnome-calculator");
    case HidTargetOs::Windows:
    default:
      return runLauncherCommand(os, "calc");
  }
}

bool HalUsbHid::openTerminal(HidTargetOs os) {
  switch (os) {
    case HidTargetOs::MacOS:
      return runLauncherCommand(os, "Terminal");
    case HidTargetOs::Linux:
      return runLauncherCommand(os, "gnome-terminal");
    case HidTargetOs::Windows:
    default:
      return runLauncherCommand(os, "cmd");
  }
}

bool HalUsbHid::runTerminalInfo(HidTargetOs os) {
  HcSerial.printf("[hid] terminal info (%s)\n", hidOsName(os));
  switch (os) {
    case HidTargetOs::Windows:
      return runLauncherCommand(os, "cmd /k ipconfig & whoami & hostname");
    case HidTargetOs::MacOS:
      if (!openTerminal(os)) {
        return false;
      }
      delay(1600);
      if (!typeText("uname -a; whoami; hostname")) {
        return false;
      }
      delay(120);
      return sendEnter();
    case HidTargetOs::Linux:
      if (!openTerminal(os)) {
        return false;
      }
      delay(1600);
      if (!typeText("uname -a; whoami; hostname; ip -4 addr")) {
        return false;
      }
      delay(120);
      return sendEnter();
    default:
      return false;
  }
}

bool HalUsbHid::typeTextPayload(const char* text) {
  _lastError[0] = '\0';
  if (!_ready || !text || text[0] == '\0') {
    strncpy(_lastError, "Text required", sizeof(_lastError) - 1);
    return false;
  }
  if (!ensureHostReady()) {
    return false;
  }
  HcSerial.printf("[hid] type text: %s\n", text);
  return typeText(text);
}

bool HalUsbHid::openWebsite(HidTargetOs os, const char* url) {
  if (!url || url[0] == '\0') {
    strncpy(_lastError, "URL required", sizeof(_lastError) - 1);
    return false;
  }

  if (os == HidTargetOs::Linux) {
    char command[160];
    snprintf(command, sizeof(command), "xdg-open %s", url);
    return runLauncherCommand(os, command);
  }

  return runLauncherCommand(os, url);
}

bool HalUsbHid::lockScreen(HidTargetOs os) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }

  HcSerial.printf("[hid] lock screen (%s)\n", hidOsName(os));
  switch (os) {
    case HidTargetOs::MacOS:
      return pressCombo(KEY_LEFT_CTRL, KEY_LEFT_GUI, 'q');
    case HidTargetOs::Linux:
    case HidTargetOs::Windows:
    default:
      return pressCombo(KEY_LEFT_GUI, 0, 'l');
  }
}

bool HalUsbHid::unlockScreen(HidTargetOs os, const char* password) {
  _lastError[0] = '\0';
  if (!password || password[0] == '\0') {
    strncpy(_lastError, "Password required", sizeof(_lastError) - 1);
    return false;
  }
  if (!ensureHostReady()) {
    return false;
  }

  HcSerial.printf("[hid] unlock screen (%s)\n", hidOsName(os));
  delay(350);
  if (!pressKey(KEY_RETURN)) {
    return false;
  }
  if (!releaseAllKeys()) {
    return false;
  }
  delay(os == HidTargetOs::MacOS ? 800 : 600);
  if (!typeTextPayload(password)) {
    return false;
  }
  delay(120);
  return sendEnter();
}

bool HalUsbHid::takeScreenshot(HidTargetOs os) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }

  HcSerial.printf("[hid] screenshot (%s)\n", hidOsName(os));
  switch (os) {
    case HidTargetOs::MacOS:
      return pressCombo(KEY_LEFT_GUI, KEY_LEFT_SHIFT, '4');
    case HidTargetOs::Linux:
      if (!pressKey(KEY_PRINT_SCREEN)) {
        return false;
      }
      return releaseAllKeys();
    case HidTargetOs::Windows:
    default:
      return pressCombo(KEY_LEFT_GUI, KEY_LEFT_SHIFT, 's');
  }
}

bool HalUsbHid::appSwitcher(HidTargetOs os) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }

  HcSerial.printf("[hid] app switcher (%s)\n", hidOsName(os));
  if (os == HidTargetOs::MacOS) {
    return pressCombo(KEY_LEFT_GUI, 0, KEY_TAB);
  }
  return pressCombo(KEY_LEFT_ALT, 0, KEY_TAB);
}

bool HalUsbHid::closeWindow(HidTargetOs os) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }

  HcSerial.printf("[hid] close window (%s)\n", hidOsName(os));
  switch (os) {
    case HidTargetOs::MacOS:
      return pressCombo(KEY_LEFT_GUI, 0, 'w');
    case HidTargetOs::Linux:
    case HidTargetOs::Windows:
    default:
      return pressCombo(KEY_LEFT_ALT, 0, KEY_F4);
  }
}

bool HalUsbHid::pressNamedKey(const char* keyName) {
  _lastError[0] = '\0';
  if (!keyName || keyName[0] == '\0') {
    strncpy(_lastError, "Key name required", sizeof(_lastError) - 1);
    return false;
  }
  if (!ensureHostReady()) {
    return false;
  }

  uint8_t key = 0;
  if (!lookupNamedKey(keyName, &key)) {
    strncpy(_lastError, "Unknown key name", sizeof(_lastError) - 1);
    return false;
  }

  HcSerial.printf("[hid] key press: %s\n", keyName);
  if (!pressKey(key)) {
    return false;
  }
  return releaseAllKeys();
}

bool HalUsbHid::volumeMute() {
  HcSerial.println("[hid] volume mute");
  return consumerPress(CONSUMER_CONTROL_MUTE);
}

bool HalUsbHid::volumeUp() {
  HcSerial.println("[hid] volume up");
  return consumerPress(CONSUMER_CONTROL_VOLUME_INCREMENT);
}

bool HalUsbHid::volumeDown() {
  HcSerial.println("[hid] volume down");
  return consumerPress(CONSUMER_CONTROL_VOLUME_DECREMENT);
}

bool HalUsbHid::mediaPlayPause() {
  HcSerial.println("[hid] media play/pause");
  return consumerPress(CONSUMER_CONTROL_PLAY_PAUSE);
}

bool HalUsbHid::mediaNext() {
  HcSerial.println("[hid] media next");
  return consumerPress(CONSUMER_CONTROL_SCAN_NEXT);
}

bool HalUsbHid::mediaPrevious() {
  HcSerial.println("[hid] media previous");
  return consumerPress(CONSUMER_CONTROL_SCAN_PREVIOUS);
}

bool HalUsbHid::mouseMove(int8_t dx, int8_t dy) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }
  if (dx == 0 && dy == 0) {
    strncpy(_lastError, "Move delta required", sizeof(_lastError) - 1);
    return false;
  }
  HcSerial.printf("[hid] mouse move %d,%d\n", dx, dy);
  sMouse.move(dx, dy, 0);
  delay(40);
  return true;
}

bool HalUsbHid::mouseClick(const char* button) {
  _lastError[0] = '\0';
  if (!ensureHostReady()) {
    return false;
  }

  uint8_t mouseButton = MOUSE_LEFT;
  if (button && button[0] != '\0') {
    if (strcmp(button, "right") == 0) {
      mouseButton = MOUSE_RIGHT;
    } else if (strcmp(button, "middle") == 0) {
      mouseButton = MOUSE_MIDDLE;
    }
  }

  HcSerial.printf("[hid] mouse click %s\n", button ? button : "left");
  sMouse.click(mouseButton);
  delay(40);
  return true;
}

#else

bool HalUsbHid::ensureHostReady() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::pressKey(uint8_t key) {
  (void)key;
  return false;
}

bool HalUsbHid::pressCombo(uint8_t mod1, uint8_t mod2, uint8_t key) {
  (void)mod1;
  (void)mod2;
  (void)key;
  return false;
}

bool HalUsbHid::releaseAllKeys() {
  return false;
}

bool HalUsbHid::consumerPress(uint16_t code) {
  (void)code;
  return false;
}

bool HalUsbHid::openLauncher(HidTargetOs os) {
  (void)os;
  return false;
}

bool HalUsbHid::pressChord(uint8_t mod1, uint8_t mod2, uint8_t key) {
  (void)mod1;
  (void)mod2;
  (void)key;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::runLauncherCommand(HidTargetOs os, const char* command) {
  (void)os;
  (void)command;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::typeText(const char* text) {
  (void)text;
  return false;
}

bool HalUsbHid::sendEnter() {
  return false;
}

bool HalUsbHid::openNotepad(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::openCalculator(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::openTerminal(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::runTerminalInfo(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::typeTextPayload(const char* text) {
  (void)text;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::openWebsite(HidTargetOs os, const char* url) {
  (void)os;
  (void)url;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::lockScreen(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::unlockScreen(HidTargetOs os, const char* password) {
  (void)os;
  (void)password;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::takeScreenshot(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::appSwitcher(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::closeWindow(HidTargetOs os) {
  (void)os;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::pressNamedKey(const char* keyName) {
  (void)keyName;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::volumeMute() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::volumeUp() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::volumeDown() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::mediaPlayPause() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::mediaNext() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::mediaPrevious() {
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::mouseMove(int8_t dx, int8_t dy) {
  (void)dx;
  (void)dy;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

bool HalUsbHid::mouseClick(const char* button) {
  (void)button;
  strncpy(_lastError, "USB HID requires USB-OTG (TinyUSB) mode", sizeof(_lastError) - 1);
  return false;
}

#endif

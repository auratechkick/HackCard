#pragma once

#include <Arduino.h>

// USB keyboard and mouse for the operator's own PC.
// RP2350 enumerates as CDC serial plus HID when these drivers start.

class HalUsbHid {
public:
  void begin();
  bool isReady() const { return _ready; }

  void typeText(const char* text);
  void tapKey(uint8_t hidKey);
  void tapEnter();
  void tapArrowRight();
  void tapArrowLeft();
  void tapConsumer(uint16_t usage);
  void volumeUp();
  void volumeDown();
  void mute();
  void mediaPlayPause();
  void mediaNext();
  void mediaPrevious();
  void moveMouse(int x, int y, int wheel = 0);
  void clickMouse();

private:
  bool _ready = false;
};

extern HalUsbHid UsbHid;

#pragma once

#include <Arduino.h>

// USB keyboard, consumer (media), and mouse for the operator's own PC (ESP32-S3 TinyUSB).

class HalUsbHid {
public:
  void begin();
  bool isReady() const { return _ready; }

  void press(uint8_t key);
  void releaseAll();
  void writeKey(uint8_t key);
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

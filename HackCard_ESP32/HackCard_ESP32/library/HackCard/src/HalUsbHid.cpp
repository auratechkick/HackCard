#include "HalUsbHid.h"

#include "USB.h"
#include "USBHIDKeyboard.h"
#include "USBHIDConsumerControl.h"
#include "USBHIDMouse.h"

namespace {
USBHIDKeyboard sKeyboard;
USBHIDConsumerControl sConsumer;
USBHIDMouse sMouse;
}  // namespace

HalUsbHid UsbHid;

void HalUsbHid::begin() {
  if (_ready) {
    return;
  }
  sKeyboard.begin();
  sConsumer.begin();
  sMouse.begin();
  USB.begin();
  _ready = true;
  Serial.println("[hid] USB keyboard + media + mouse ready");
}

void HalUsbHid::press(uint8_t key) {
  if (_ready) {
    sKeyboard.press(key);
  }
}

void HalUsbHid::releaseAll() {
  if (_ready) {
    sKeyboard.releaseAll();
  }
}

void HalUsbHid::writeKey(uint8_t key) {
  if (_ready) {
    sKeyboard.write(key);
  }
}

void HalUsbHid::typeText(const char* text) {
  if (!_ready || !text) {
    return;
  }
  sKeyboard.print(text);
}

void HalUsbHid::tapKey(uint8_t hidKey) {
  if (!_ready) {
    return;
  }
  sKeyboard.press(hidKey);
  delay(20);
  sKeyboard.release(hidKey);
}

void HalUsbHid::tapEnter() {
  tapKey(KEY_RETURN);
}

void HalUsbHid::tapArrowRight() {
  tapKey(KEY_RIGHT_ARROW);
}

void HalUsbHid::tapArrowLeft() {
  tapKey(KEY_LEFT_ARROW);
}

void HalUsbHid::tapConsumer(uint16_t usage) {
  if (!_ready) {
    return;
  }
  sConsumer.press(usage);
  delay(20);
  sConsumer.release();
}

void HalUsbHid::volumeUp() {
  tapConsumer(CONSUMER_CONTROL_VOLUME_INCREMENT);
}

void HalUsbHid::volumeDown() {
  tapConsumer(CONSUMER_CONTROL_VOLUME_DECREMENT);
}

void HalUsbHid::mute() {
  tapConsumer(CONSUMER_CONTROL_MUTE);
}

void HalUsbHid::mediaPlayPause() {
  tapConsumer(CONSUMER_CONTROL_PLAY_PAUSE);
}

void HalUsbHid::mediaNext() {
  tapConsumer(CONSUMER_CONTROL_SCAN_NEXT);
}

void HalUsbHid::mediaPrevious() {
  tapConsumer(CONSUMER_CONTROL_SCAN_PREVIOUS);
}

void HalUsbHid::moveMouse(int x, int y, int wheel) {
  if (!_ready) {
    return;
  }
  sMouse.move(x, y, wheel);
}

void HalUsbHid::clickMouse() {
  if (!_ready) {
    return;
  }
  sMouse.click(MOUSE_BUTTON_LEFT);
}

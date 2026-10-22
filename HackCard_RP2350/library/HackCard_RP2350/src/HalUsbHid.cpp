#include "HalUsbHid.h"

#include <Keyboard.h>
#include <Mouse.h>

HalUsbHid UsbHid;

void HalUsbHid::begin() {
  Keyboard.begin();
  Mouse.begin();
  _ready = true;
  Serial.println("[hid] USB keyboard + mouse ready");
}

void HalUsbHid::typeText(const char* text) {
  if (!_ready || !text) {
    return;
  }
  Keyboard.print(text);
}

void HalUsbHid::tapKey(uint8_t hidKey) {
  if (!_ready) {
    return;
  }
  Keyboard.press(hidKey);
  delay(20);
  Keyboard.release(hidKey);
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
  Keyboard.consumerPress(usage);
  delay(20);
  Keyboard.consumerRelease();
}

void HalUsbHid::volumeUp() {
  tapConsumer(KEY_VOLUME_INCREMENT);
}

void HalUsbHid::volumeDown() {
  tapConsumer(KEY_VOLUME_DECREMENT);
}

void HalUsbHid::mute() {
  tapConsumer(KEY_MUTE);
}

void HalUsbHid::mediaPlayPause() {
  tapConsumer(KEY_PLAY_PAUSE);
}

void HalUsbHid::mediaNext() {
  tapConsumer(KEY_SCAN_NEXT);
}

void HalUsbHid::mediaPrevious() {
  tapConsumer(KEY_SCAN_PREVIOUS);
}

void HalUsbHid::moveMouse(int x, int y, int wheel) {
  if (!_ready) {
    return;
  }
  Mouse.move(x, y, wheel);
}

void HalUsbHid::clickMouse() {
  if (!_ready) {
    return;
  }
  Mouse.click(MOUSE_LEFT);
}

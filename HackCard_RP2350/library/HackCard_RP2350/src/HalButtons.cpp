#include "HalButtons.h"

#include "HackCard_Pins.h"

HalButtons Buttons;

void HalButtons::begin() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void HalButtons::update() {
  const bool down = digitalRead(PIN_BUTTON) == LOW;
  const uint32_t now = millis();

  if (down && !_pressed) {
    _pressed = true;
    _longHandled = false;
    _pressStartedMs = now;
  }

  if (down && _pressed && !_longHandled && (now - _pressStartedMs >= BOOT_LONG_PRESS_MS)) {
    _pending = ButtonGesture::Long;
    _longHandled = true;
    _tapCount = 0;
  }

  if (!down && _pressed) {
    _pressed = false;
    const uint32_t held = now - _pressStartedMs;

    if (!_longHandled && held < BOOT_LONG_PRESS_MS) {
      _tapCount++;
      _lastReleaseMs = now;
    }
  }

  if (_tapCount > 0 && !_pressed && (now - _lastReleaseMs > BOOT_DOUBLE_TAP_MS)) {
    if (_tapCount >= 2) {
      _pending = ButtonGesture::Double;
    } else if (!_longHandled) {
      _pending = ButtonGesture::Short;
    }
    _tapCount = 0;
  }
}

ButtonGesture HalButtons::consumeGesture() {
  const ButtonGesture gesture = _pending;
  _pending = ButtonGesture::None;
  return gesture;
}

bool HalButtons::isBootHeld(uint32_t holdMs) const {
  return _pressed && ((millis() - _pressStartedMs) >= holdMs);
}

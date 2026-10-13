#pragma once

#include <Arduino.h>

enum class ButtonGesture {
  None,
  Short,
  Long,
  Double
};

class HalButtons {
public:
  void begin();
  void update();
  ButtonGesture consumeGesture();
  bool isBootHeld(uint32_t holdMs) const;

private:
  bool _pressed = false;
  bool _longHandled = false;
  uint32_t _pressStartedMs = 0;
  uint32_t _lastReleaseMs = 0;
  uint8_t _tapCount = 0;
  ButtonGesture _pending = ButtonGesture::None;
};

extern HalButtons Buttons;

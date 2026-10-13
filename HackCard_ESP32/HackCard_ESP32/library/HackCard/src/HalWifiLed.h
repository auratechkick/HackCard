#pragma once

#include <Arduino.h>

enum class WifiLedState {
  Off,
  ApActive,
  StaConnected,
  Scanning,
  Error,
  DataTransfer
};

class HalWifiLed {
public:
  void begin(uint8_t brightness);
  void setBrightness(uint8_t brightness);
  void setState(WifiLedState state);
  void update();

private:
  void render();
  uint32_t color(uint8_t r, uint8_t g, uint8_t b) const;

  WifiLedState _state = WifiLedState::Off;
  uint8_t _brightness = 80;
  bool _pulseOn = false;
  uint32_t _lastAnimMs = 0;
};

extern HalWifiLed WifiLed;

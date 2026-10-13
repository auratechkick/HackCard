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
  /** pattern: 0=pulse, 1=solid, 2=strobe */
  void setTheme(uint8_t r, uint8_t g, uint8_t b, uint8_t pattern);
  void update();

private:
  void render();
  uint32_t color(uint8_t r, uint8_t g, uint8_t b) const;

  WifiLedState _state = WifiLedState::Off;
  uint8_t _brightness = 80;
  uint8_t _themeR = 0;
  uint8_t _themeG = 180;
  uint8_t _themeB = 60;
  uint8_t _themePattern = 0;
  bool _pulseOn = false;
  uint32_t _lastAnimMs = 0;
};

extern HalWifiLed WifiLed;

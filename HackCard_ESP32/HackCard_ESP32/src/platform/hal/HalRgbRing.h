#pragma once

#include <Arduino.h>

enum class RingPattern {
  Off,
  BootAnim,
  Idle,
  Scanning,
  Success,
  Error,
  Pointer,
  FullMode,
  FlashMode
};

class HalRgbRing {
public:
  void begin(uint8_t brightness);
  void setBrightness(uint8_t brightness);
  void setPattern(RingPattern pattern);
  void setPointer(uint8_t index);
  void setProgress(uint8_t percent);
  void showColor(uint8_t r, uint8_t g, uint8_t b);
  void startDemo(uint8_t patternId, uint8_t r, uint8_t g, uint8_t b);
  void stopDemo();
  bool isDemoActive() const { return _demoActive; }
  /** Wi-Fi lab ring look when scanning (0 = built-in chase). */
  void setWifiScanStyle(uint8_t patternId, uint8_t r, uint8_t g, uint8_t b);
  void applyWifiScanLook();
  void update();

  bool bootAnimationDone() const { return _bootDone; }

private:
  void render();
  void renderDemo();
  uint32_t scaleColor(uint8_t r, uint8_t g, uint8_t b) const;
  uint32_t scaleColorDim(uint8_t r, uint8_t g, uint8_t b, uint8_t scale) const;
  uint32_t wheel(byte pos) const;

  RingPattern _pattern = RingPattern::Idle;
  bool _demoActive = false;
  uint8_t _demoPattern = 1;
  uint8_t _demoR = 0;
  uint8_t _demoG = 120;
  uint8_t _demoB = 255;
  uint8_t _brightness = 80;
  uint8_t _pointer = 0;
  uint8_t _progress = 0;
  uint32_t _lastAnimMs = 0;
  uint16_t _animStep = 0;
  bool _bootDone = false;
  uint8_t _wifiScanPattern = 0;
  uint8_t _wifiScanR = 0;
  uint8_t _wifiScanG = 40;
  uint8_t _wifiScanB = 255;
};

extern HalRgbRing RgbRing;

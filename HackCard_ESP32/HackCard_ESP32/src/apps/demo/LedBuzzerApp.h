#pragma once

#include <Arduino.h>

struct HackCardConfig;

class LedBuzzerApp {
public:
  void applySettings(const HackCardConfig& config);
  bool playRing(const char* pattern);
  bool playRingDemo(uint8_t patternId, uint8_t r, uint8_t g, uint8_t b);
  bool stopRingDemo();
  bool playBuzzer(const char* tone);
  bool playBuzzerTune(uint8_t tuneId);
  bool playBuzzerDemo(uint8_t demoId);
  bool stopBuzzerMelody();
  bool isBuzzerMelodyPlaying() const;
  bool setRingBrightness(uint8_t brightness, HackCardConfig& config);
  bool setBuzzerVolume(uint8_t volume, HackCardConfig& config);
};

extern LedBuzzerApp LedBuzzer;

String buildVcard(const HackCardConfig& config);

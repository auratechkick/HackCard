#pragma once

#include <Arduino.h>

enum class BuzzerTuneId : uint8_t {
  Classic = 0,
  Soft,
  Alert,
  Arcade,
  Chime,
  Pulse,
  Warning,
  Digital,
  Count
};

class HalBuzzer {
public:
  void begin(bool enabled);
  void setEnabled(bool enabled);
  void setTunes(uint8_t success, uint8_t error, uint8_t boot, uint8_t nfc);

  void playSuccess();
  void playError();
  void playBoot();
  void playNfcReady();
  void playNfcWriteOk();
  void playNfcCloneOk();
  void playNfcKillOk();
  void playTune(BuzzerTuneId tune);
  void stop();

  bool isEnabled() const { return _enabled; }
  static const char* tuneName(uint8_t id);

private:
  void attachTone(uint16_t frequency);
  void detachTone();
  void toneFor(uint16_t frequency, uint16_t durationMs);
  void playTuneInternal(BuzzerTuneId tune);

  bool _enabled = true;
  bool _attached = false;
  uint8_t _tuneSuccess = 0;
  uint8_t _tuneError = 0;
  uint8_t _tuneBoot = 0;
  uint8_t _tuneNfc = 0;
};

extern HalBuzzer Buzzer;

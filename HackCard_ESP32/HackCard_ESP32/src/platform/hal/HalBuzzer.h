#pragma once

#include <Arduino.h>

struct BuzzerMelodyStep {
  uint16_t freq;
  uint16_t ms;
};

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
  void setVolume(uint8_t volumePercent);
  uint8_t getVolume() const { return _volume; }
  void setTunes(uint8_t success, uint8_t error, uint8_t boot, uint8_t nfc);

  void playSuccess();
  void playError();
  void playBoot();
  void playNfcReady();
  void playNfcWriteOk();
  void playNfcCloneOk();
  void playNfcKillOk();
  void playTune(BuzzerTuneId tune);

  /** Song previews for lab UI (ids 0–4). Runs in background; use stopSongMelody(). */
  bool startSongMelodyAsync(uint8_t demoId);
  void stopSongMelody();
  bool isSongMelodyPlaying() const;

  static const char* songMelodyTitle(uint8_t demoId);
  void stop();

  bool isEnabled() const { return _enabled; }
  static const char* tuneName(uint8_t id);

private:
  static void songMelodyTaskEntry(void* param);
  void playSongMelodyBlocking(uint8_t demoId);

  void attachTone(uint16_t frequency);
  void detachTone();
  void applyPwmDuty();
  void tone(uint16_t frequency, uint16_t durationMs);
  void toneMelody(uint16_t frequency, uint16_t durationMs);
  bool delayUnlessStopped(uint32_t ms);
  void playTuneInternal(BuzzerTuneId tune);
  void playMelodySteps(const BuzzerMelodyStep* steps, size_t count);

  bool _enabled = true;
  bool _attached = false;
  bool _melodyPlayback = false;
  volatile bool _stopMelodyRequested = false;
  uint8_t _volume = 80;
  uint8_t _tuneSuccess = 0;
  uint8_t _tuneError = 0;
  uint8_t _tuneBoot = 0;
  uint8_t _tuneNfc = 0;
};

extern HalBuzzer Buzzer;

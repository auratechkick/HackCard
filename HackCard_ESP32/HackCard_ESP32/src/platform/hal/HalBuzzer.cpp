#include "HalBuzzer.h"

#include "config/board_config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

HalBuzzer Buzzer;

namespace {

TaskHandle_t gSongMelodyTask = nullptr;
uint8_t gSongMelodyId = 0;

}  // namespace

namespace {

constexpr uint8_t kMaxTune = static_cast<uint8_t>(BuzzerTuneId::Count);

// Note frequencies (Hz) — C4 octave for kid songs, mixed for classics.
constexpr uint16_t kC4 = 262;
constexpr uint16_t kD4 = 294;
constexpr uint16_t kE4 = 330;
constexpr uint16_t kF4 = 349;
constexpr uint16_t kG4 = 392;
constexpr uint16_t kA4 = 440;
constexpr uint16_t kB4 = 494;
constexpr uint16_t kC5 = 523;
constexpr uint16_t kD5 = 587;
constexpr uint16_t kE5 = 659;
constexpr uint16_t kF5 = 698;
constexpr uint16_t kG5 = 784;
constexpr uint16_t kA5 = 880;
constexpr uint16_t kB5 = 988;
constexpr uint16_t kDs5 = 622;
constexpr uint16_t kDs4 = 311;
constexpr uint16_t kFs4 = 370;
constexpr uint16_t kAs4 = 466;

constexpr uint16_t kQ = 460;
constexpr uint16_t kH = 920;
constexpr uint16_t kEighth = 230;

#define REST(ms) \
  { 0, ms }
#define NOTE(f, ms) \
  { f, ms }

// Twinkle, Twinkle, Little Star (public domain) — ~17 s
const BuzzerMelodyStep kSongTwinkle[] = {
    NOTE(kC4, kQ), NOTE(kC4, kQ), NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kA4, kQ), NOTE(kA4, kQ), NOTE(kG4, kH),
    NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kD4, kQ), NOTE(kD4, kQ), NOTE(kC4, kH),
    NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kD4, kH),
    NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kD4, kH),
    NOTE(kC4, kQ), NOTE(kC4, kQ), NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kA4, kQ), NOTE(kA4, kQ), NOTE(kG4, kH),
    NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kD4, kQ), NOTE(kD4, kQ), NOTE(kC4, kH),
    REST(200),
};

// Happy Birthday (traditional) — ~14 s
const BuzzerMelodyStep kSongHappyBirthday[] = {
    NOTE(kC4, kEighth), NOTE(kC4, kEighth), NOTE(kD4, kQ), NOTE(kC4, kQ), NOTE(kF4, kQ), NOTE(kE4, kH),
    NOTE(kC4, kEighth), NOTE(kC4, kEighth), NOTE(kD4, kQ), NOTE(kC4, kQ), NOTE(kG4, kQ), NOTE(kF4, kH),
    NOTE(kC4, kEighth), NOTE(kC4, kEighth), NOTE(kC5, kQ), NOTE(kA4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ),
    NOTE(kD4, kH),
    NOTE(kAs4, kEighth), NOTE(kAs4, kEighth), NOTE(kA4, kQ), NOTE(kF4, kQ), NOTE(kG4, kQ), NOTE(kF4, kH),
    REST(200),
};

// Für Elise — opening motif (Beethoven, public domain) — ~16 s
const BuzzerMelodyStep kSongFurElise[] = {
    NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth),
    NOTE(kB4, kEighth), NOTE(kD5, kQ), NOTE(kC5, kQ), NOTE(kA4, kH),
    REST(kEighth),
    NOTE(kC4, kEighth), NOTE(kE4, kEighth), NOTE(kA4, kQ), NOTE(kG4, kEighth), NOTE(kFs4, kEighth),
    NOTE(kG4, kEighth), NOTE(kA4, kH),
    NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth),
    NOTE(kB4, kEighth), NOTE(kD5, kQ), NOTE(kC5, kQ), NOTE(kA4, kH),
    NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth), NOTE(kDs5, kEighth), NOTE(kE5, kEighth),
    NOTE(kB4, kEighth), NOTE(kD5, kQ), NOTE(kC5, kQ), NOTE(kA4, kH),
    REST(300),
};

// Ode to Joy — theme (Beethoven, public domain) — ~18 s
const BuzzerMelodyStep kSongOdeToJoy[] = {
    NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kF4, kQ), NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ),
    NOTE(kD4, kQ), NOTE(kC4, kQ), NOTE(kC4, kQ), NOTE(kD4, kQ), NOTE(kE4, kQ), NOTE(kE4, kEighth),
    NOTE(kD4, kEighth), NOTE(kD4, kH),
    NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kF4, kQ), NOTE(kG4, kQ), NOTE(kG4, kQ), NOTE(kF4, kQ), NOTE(kE4, kQ),
    NOTE(kD4, kQ), NOTE(kC4, kQ), NOTE(kC4, kQ), NOTE(kD4, kQ), NOTE(kE4, kQ), NOTE(kD4, kEighth),
    NOTE(kC4, kEighth), NOTE(kC4, kH),
    REST(250),
};

// Jingle Bells — chorus (public domain) — ~15 s
const BuzzerMelodyStep kSongJingleBells[] = {
    NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kE4, kH),
    NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kE4, kH),
    NOTE(kE4, kQ), NOTE(kG4, kQ), NOTE(kC4, kQ), NOTE(kD4, kQ), NOTE(kE4, kH + kQ),
    NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kF4, kQ), NOTE(kF4, kH),
    NOTE(kF4, kQ), NOTE(kE4, kQ), NOTE(kE4, kQ), NOTE(kE4, kEighth), NOTE(kE4, kEighth),
    NOTE(kE4, kQ), NOTE(kD4, kQ), NOTE(kD4, kQ), NOTE(kE4, kQ), NOTE(kD4, kH), NOTE(kG4, kH),
    REST(200),
};

#undef NOTE
#undef REST

}  // namespace

const char* HalBuzzer::tuneName(uint8_t id) {
  switch (static_cast<BuzzerTuneId>(id >= kMaxTune ? 0 : id)) {
    case BuzzerTuneId::Soft:
      return "Soft";
    case BuzzerTuneId::Alert:
      return "Alert";
    case BuzzerTuneId::Arcade:
      return "Arcade";
    case BuzzerTuneId::Chime:
      return "Chime";
    case BuzzerTuneId::Pulse:
      return "Pulse";
    case BuzzerTuneId::Warning:
      return "Warning";
    case BuzzerTuneId::Digital:
      return "Digital";
    default:
      return "Classic";
  }
}

void HalBuzzer::begin(bool enabled) {
  _enabled = enabled;
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
}

void HalBuzzer::setEnabled(bool enabled) {
  _enabled = enabled;
  if (!_enabled) {
    stop();
  }
}

void HalBuzzer::setVolume(uint8_t volumePercent) {
  if (volumePercent < 10) {
    volumePercent = 10;
  }
  if (volumePercent > 100) {
    volumePercent = 100;
  }
  _volume = volumePercent;
}

void HalBuzzer::setTunes(uint8_t success, uint8_t error, uint8_t boot, uint8_t nfc) {
  _tuneSuccess = success >= kMaxTune ? 0 : success;
  _tuneError = error >= kMaxTune ? 0 : error;
  _tuneBoot = boot >= kMaxTune ? 0 : boot;
  _tuneNfc = nfc >= kMaxTune ? 0 : nfc;
}

void HalBuzzer::attachTone(uint16_t frequency) {
  if (_attached) {
    ledcDetach(PIN_BUZZER);
    _attached = false;
  }
  ledcAttach(PIN_BUZZER, frequency, 8);
  _attached = true;
}

void HalBuzzer::detachTone() {
  if (_attached) {
    ledcDetach(PIN_BUZZER);
    _attached = false;
  }
  pinMode(PIN_BUZZER, INPUT);
}

void HalBuzzer::applyPwmDuty() {
  if (!_attached) {
    return;
  }
  uint8_t v = _volume;
  if (v < 10) {
    v = 10;
  }
  if (v > 100) {
    v = 100;
  }
  // Songs use a lower PWM ceiling so the volume slider matches long melodies (chimes stay brighter).
  const uint32_t minDuty = _melodyPlayback ? 2UL : 8UL;
  const uint32_t maxDuty = _melodyPlayback ? 72UL : 120UL;
  const uint32_t duty = minDuty + ((maxDuty - minDuty) * static_cast<uint32_t>(v - 10)) / 90UL;
  ledcWrite(PIN_BUZZER, duty);
}

bool HalBuzzer::delayUnlessStopped(uint32_t ms) {
  constexpr uint32_t kSlice = 12;
  uint32_t elapsed = 0;
  while (elapsed < ms) {
    if (_stopMelodyRequested) {
      return false;
    }
    const uint32_t slice = (ms - elapsed) > kSlice ? kSlice : (ms - elapsed);
    delay(slice);
    elapsed += slice;
  }
  return !_stopMelodyRequested;
}

void HalBuzzer::tone(uint16_t frequency, uint16_t durationMs) {
  if (!_enabled) {
    return;
  }

  attachTone(frequency);
  applyPwmDuty();
  delay(durationMs);
  stop();
}

void HalBuzzer::toneMelody(uint16_t frequency, uint16_t durationMs) {
  if (!_enabled || _stopMelodyRequested) {
    return;
  }

  attachTone(frequency);
  applyPwmDuty();
  if (!delayUnlessStopped(durationMs)) {
    stop();
    return;
  }
  stop();
}

void HalBuzzer::playMelodySteps(const BuzzerMelodyStep* steps, size_t count) {
  if (!_enabled || steps == nullptr || count == 0) {
    return;
  }

  _melodyPlayback = true;
  _stopMelodyRequested = false;

  for (size_t i = 0; i < count; ++i) {
    if (_stopMelodyRequested) {
      break;
    }
    if (steps[i].freq == 0) {
      if (steps[i].ms > 0 && !delayUnlessStopped(steps[i].ms)) {
        break;
      }
      continue;
    }
    toneMelody(steps[i].freq, steps[i].ms);
  }

  _melodyPlayback = false;
  stop();
}

bool HalBuzzer::isSongMelodyPlaying() const {
  return gSongMelodyTask != nullptr;
}

void HalBuzzer::songMelodyTaskEntry(void* param) {
  HalBuzzer* self = static_cast<HalBuzzer*>(param);
  const uint8_t songId = gSongMelodyId;
  if (self) {
    self->playSongMelodyBlocking(songId);
  }
  gSongMelodyTask = nullptr;
  vTaskDelete(nullptr);
}

bool HalBuzzer::startSongMelodyAsync(uint8_t demoId) {
  if (!_enabled) {
    return false;
  }
  if (gSongMelodyTask != nullptr) {
    stopSongMelody();
    delay(20);
  }
  if (demoId > 4) {
    demoId = 0;
  }
  gSongMelodyId = demoId;
  _stopMelodyRequested = false;

  if (xTaskCreatePinnedToCore(songMelodyTaskEntry, "hcSong", 4096, this, 1, &gSongMelodyTask, 1) != pdPASS) {
    gSongMelodyTask = nullptr;
    return false;
  }
  return true;
}

void HalBuzzer::stopSongMelody() {
  _stopMelodyRequested = true;
  stop();
}

void HalBuzzer::playSongMelodyBlocking(uint8_t demoId) {
  if (!_enabled) {
    return;
  }

  switch (demoId) {
    case 0:
      playMelodySteps(kSongTwinkle, sizeof(kSongTwinkle) / sizeof(kSongTwinkle[0]));
      break;
    case 1:
      playMelodySteps(kSongHappyBirthday, sizeof(kSongHappyBirthday) / sizeof(kSongHappyBirthday[0]));
      break;
    case 2:
      playMelodySteps(kSongFurElise, sizeof(kSongFurElise) / sizeof(kSongFurElise[0]));
      break;
    case 3:
      playMelodySteps(kSongOdeToJoy, sizeof(kSongOdeToJoy) / sizeof(kSongOdeToJoy[0]));
      break;
    case 4:
      playMelodySteps(kSongJingleBells, sizeof(kSongJingleBells) / sizeof(kSongJingleBells[0]));
      break;
    default:
      playMelodySteps(kSongTwinkle, sizeof(kSongTwinkle) / sizeof(kSongTwinkle[0]));
      break;
  }
}

const char* HalBuzzer::songMelodyTitle(uint8_t demoId) {
  switch (demoId) {
    case 0:
      return "Twinkle, Twinkle";
    case 1:
      return "Happy Birthday";
    case 2:
      return "Fur Elise (intro)";
    case 3:
      return "Ode to Joy";
    case 4:
      return "Jingle Bells";
    default:
      return "Song";
  }
}

void HalBuzzer::stop() {
  if (_attached) {
    ledcWrite(PIN_BUZZER, 0);
  }
  detachTone();
}

void HalBuzzer::playTuneInternal(BuzzerTuneId tune) {
  if (!_enabled) {
    return;
  }

  switch (tune) {
    case BuzzerTuneId::Soft:
      tone(523, 70);
      tone(659, 90);
      break;
    case BuzzerTuneId::Alert:
      tone(988, 60);
      tone(988, 60);
      tone(988, 80);
      break;
    case BuzzerTuneId::Arcade:
      tone(523, 50);
      tone(659, 50);
      tone(784, 50);
      tone(1046, 80);
      break;
    case BuzzerTuneId::Chime:
      tone(784, 80);
      tone(988, 80);
      tone(1175, 120);
      break;
    case BuzzerTuneId::Pulse:
      tone(880, 120);
      break;
    case BuzzerTuneId::Warning:
      tone(330, 100);
      tone(220, 140);
      tone(330, 100);
      break;
    case BuzzerTuneId::Digital:
      tone(1200, 40);
      tone(900, 40);
      tone(1200, 60);
      break;
    case BuzzerTuneId::Classic:
    default:
      tone(880, 80);
      tone(1175, 120);
      break;
  }
}

void HalBuzzer::playTune(BuzzerTuneId tune) {
  playTuneInternal(tune);
}

void HalBuzzer::playSuccess() {
  playTuneInternal(static_cast<BuzzerTuneId>(_tuneSuccess));
}

void HalBuzzer::playError() {
  switch (static_cast<BuzzerTuneId>(_tuneError)) {
    case BuzzerTuneId::Soft:
      tone(392, 120);
      break;
    case BuzzerTuneId::Alert:
      tone(220, 80);
      tone(220, 80);
      break;
    case BuzzerTuneId::Arcade:
      tone(784, 60);
      tone(523, 100);
      break;
    case BuzzerTuneId::Chime:
      tone(622, 100);
      tone(494, 140);
      break;
    case BuzzerTuneId::Pulse:
      tone(220, 150);
      break;
    case BuzzerTuneId::Warning:
      tone(220, 180);
      tone(165, 200);
      break;
    case BuzzerTuneId::Digital:
      tone(600, 50);
      tone(400, 80);
      break;
    default:
      tone(220, 180);
      break;
  }
}

void HalBuzzer::playBoot() {
  switch (static_cast<BuzzerTuneId>(_tuneBoot)) {
    case BuzzerTuneId::Soft:
      tone(440, 50);
      tone(554, 70);
      break;
    case BuzzerTuneId::Alert:
      tone(660, 40);
      tone(880, 40);
      tone(660, 40);
      break;
    case BuzzerTuneId::Arcade:
      tone(392, 40);
      tone(523, 40);
      tone(659, 40);
      tone(784, 60);
      break;
    case BuzzerTuneId::Chime:
      tone(659, 60);
      tone(784, 60);
      tone(988, 80);
      break;
    case BuzzerTuneId::Pulse:
      tone(660, 100);
      break;
    case BuzzerTuneId::Warning:
      tone(440, 80);
      tone(330, 100);
      break;
    case BuzzerTuneId::Digital:
      tone(1000, 30);
      tone(800, 30);
      tone(1000, 50);
      break;
    default:
      tone(440, 60);
      tone(660, 60);
      tone(880, 80);
      break;
  }
}

void HalBuzzer::playNfcReady() {
  playTuneInternal(static_cast<BuzzerTuneId>(_tuneNfc));
}

void HalBuzzer::playNfcWriteOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Soft:
      tone(659, 80);
      tone(784, 100);
      break;
    case BuzzerTuneId::Alert:
      tone(880, 50);
      tone(1046, 70);
      break;
    case BuzzerTuneId::Arcade:
      tone(523, 40);
      tone(784, 60);
      tone(1046, 80);
      break;
    case BuzzerTuneId::Chime:
      tone(988, 70);
      tone(1175, 90);
      break;
    case BuzzerTuneId::Pulse:
      tone(1046, 100);
      break;
    case BuzzerTuneId::Warning:
      tone(587, 90);
      tone(440, 110);
      break;
    case BuzzerTuneId::Digital:
      tone(1100, 35);
      tone(880, 35);
      tone(1100, 55);
      break;
    default:
      tone(880, 90);
      tone(1046, 110);
      break;
  }
}

void HalBuzzer::playNfcCloneOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Arcade:
      tone(523, 50);
      tone(659, 50);
      tone(784, 50);
      tone(988, 70);
      break;
    case BuzzerTuneId::Digital:
      tone(900, 35);
      tone(1100, 35);
      tone(900, 35);
      tone(1100, 55);
      break;
    default:
      tone(660, 70);
      tone(880, 70);
      tone(1175, 100);
      break;
  }
}

void HalBuzzer::playNfcKillOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Soft:
      tone(392, 100);
      tone(294, 120);
      break;
    case BuzzerTuneId::Warning:
      tone(294, 120);
      tone(196, 160);
      break;
    default:
      tone(330, 120);
      tone(220, 180);
      break;
  }
}

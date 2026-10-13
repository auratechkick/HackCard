#include "HalBuzzer.h"

#include "HackCard_Pins.h"

HalBuzzer Buzzer;

namespace {

constexpr uint8_t kMaxTune = static_cast<uint8_t>(BuzzerTuneId::Count);

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

void HalBuzzer::tone(uint16_t frequency, uint16_t durationMs) {
  if (!_enabled) {
    return;
  }

  attachTone(frequency);
  ledcWriteTone(PIN_BUZZER, frequency);
  delay(durationMs);
  stop();
}

void HalBuzzer::stop() {
  if (_attached) {
    ledcWriteTone(PIN_BUZZER, 0);
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

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
  ::tone(PIN_BUZZER, frequency);
  _attached = true;
}

void HalBuzzer::detachTone() {
  ::noTone(PIN_BUZZER);
  _attached = false;
  digitalWrite(PIN_BUZZER, LOW);
}

void HalBuzzer::toneFor(uint16_t frequency, uint16_t durationMs) {
  if (!_enabled) {
    return;
  }

  attachTone(frequency);
  delay(durationMs);
  stop();
}

void HalBuzzer::stop() {
  detachTone();
}

void HalBuzzer::playTuneInternal(BuzzerTuneId tune) {
  if (!_enabled) {
    return;
  }

  switch (tune) {
    case BuzzerTuneId::Soft:
      toneFor(523, 70);
      toneFor(659, 90);
      break;
    case BuzzerTuneId::Alert:
      toneFor(988, 60);
      toneFor(988, 60);
      toneFor(988, 80);
      break;
    case BuzzerTuneId::Arcade:
      toneFor(523, 50);
      toneFor(659, 50);
      toneFor(784, 50);
      toneFor(1046, 80);
      break;
    case BuzzerTuneId::Chime:
      toneFor(784, 80);
      toneFor(988, 80);
      toneFor(1175, 120);
      break;
    case BuzzerTuneId::Pulse:
      toneFor(880, 120);
      break;
    case BuzzerTuneId::Warning:
      toneFor(330, 100);
      toneFor(220, 140);
      toneFor(330, 100);
      break;
    case BuzzerTuneId::Digital:
      toneFor(1200, 40);
      toneFor(900, 40);
      toneFor(1200, 60);
      break;
    case BuzzerTuneId::Classic:
    default:
      toneFor(880, 80);
      toneFor(1175, 120);
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
      toneFor(392, 120);
      break;
    case BuzzerTuneId::Alert:
      toneFor(220, 80);
      toneFor(220, 80);
      break;
    case BuzzerTuneId::Arcade:
      toneFor(784, 60);
      toneFor(523, 100);
      break;
    case BuzzerTuneId::Chime:
      toneFor(622, 100);
      toneFor(494, 140);
      break;
    case BuzzerTuneId::Pulse:
      toneFor(220, 150);
      break;
    case BuzzerTuneId::Warning:
      toneFor(220, 180);
      toneFor(165, 200);
      break;
    case BuzzerTuneId::Digital:
      toneFor(600, 50);
      toneFor(400, 80);
      break;
    default:
      toneFor(220, 180);
      break;
  }
}

void HalBuzzer::playBoot() {
  switch (static_cast<BuzzerTuneId>(_tuneBoot)) {
    case BuzzerTuneId::Soft:
      toneFor(440, 50);
      toneFor(554, 70);
      break;
    case BuzzerTuneId::Alert:
      toneFor(660, 40);
      toneFor(880, 40);
      toneFor(660, 40);
      break;
    case BuzzerTuneId::Arcade:
      toneFor(392, 40);
      toneFor(523, 40);
      toneFor(659, 40);
      toneFor(784, 60);
      break;
    case BuzzerTuneId::Chime:
      toneFor(659, 60);
      toneFor(784, 60);
      toneFor(988, 80);
      break;
    case BuzzerTuneId::Pulse:
      toneFor(660, 100);
      break;
    case BuzzerTuneId::Warning:
      toneFor(440, 80);
      toneFor(330, 100);
      break;
    case BuzzerTuneId::Digital:
      toneFor(1000, 30);
      toneFor(800, 30);
      toneFor(1000, 50);
      break;
    default:
      toneFor(440, 60);
      toneFor(660, 60);
      toneFor(880, 80);
      break;
  }
}

void HalBuzzer::playNfcReady() {
  playTuneInternal(static_cast<BuzzerTuneId>(_tuneNfc));
}

void HalBuzzer::playNfcWriteOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Soft:
      toneFor(659, 80);
      toneFor(784, 100);
      break;
    case BuzzerTuneId::Alert:
      toneFor(880, 50);
      toneFor(1046, 70);
      break;
    case BuzzerTuneId::Arcade:
      toneFor(523, 40);
      toneFor(784, 60);
      toneFor(1046, 80);
      break;
    case BuzzerTuneId::Chime:
      toneFor(988, 70);
      toneFor(1175, 90);
      break;
    case BuzzerTuneId::Pulse:
      toneFor(1046, 100);
      break;
    case BuzzerTuneId::Warning:
      toneFor(587, 90);
      toneFor(440, 110);
      break;
    case BuzzerTuneId::Digital:
      toneFor(1100, 35);
      toneFor(880, 35);
      toneFor(1100, 55);
      break;
    default:
      toneFor(880, 90);
      toneFor(1046, 110);
      break;
  }
}

void HalBuzzer::playNfcCloneOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Arcade:
      toneFor(523, 50);
      toneFor(659, 50);
      toneFor(784, 50);
      toneFor(988, 70);
      break;
    case BuzzerTuneId::Digital:
      toneFor(900, 35);
      toneFor(1100, 35);
      toneFor(900, 35);
      toneFor(1100, 55);
      break;
    default:
      toneFor(660, 70);
      toneFor(880, 70);
      toneFor(1175, 100);
      break;
  }
}

void HalBuzzer::playNfcKillOk() {
  switch (static_cast<BuzzerTuneId>(_tuneNfc)) {
    case BuzzerTuneId::Soft:
      toneFor(392, 100);
      toneFor(294, 120);
      break;
    case BuzzerTuneId::Warning:
      toneFor(294, 120);
      toneFor(196, 160);
      break;
    default:
      toneFor(330, 120);
      toneFor(220, 180);
      break;
  }
}

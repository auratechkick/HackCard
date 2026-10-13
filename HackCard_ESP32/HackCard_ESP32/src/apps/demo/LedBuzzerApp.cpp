#include "LedBuzzerApp.h"

#include "../../core/AppContext.h"
#include "../../core/BootState.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalRgbRing.h"
#include "../../platform/hal/HalWifiLed.h"
#include "../../platform/telemetry/StatusLed.h"

LedBuzzerApp LedBuzzer;

void LedBuzzerApp::applySettings(const HackCardConfig& config) {
  if (!hackcardPeripheralsReady()) {
    return;
  }

  RgbRing.setBrightness(config.ringBrightness);
  WifiLed.setBrightness(config.ringBrightness);
  WifiLed.setTheme(config.wifiLedR, config.wifiLedG, config.wifiLedB, config.wifiLedPattern);
  RgbRing.setWifiScanStyle(config.wifiRingPattern, config.wifiRingR, config.wifiRingG, config.wifiRingB);
  Buzzer.setEnabled(config.buzzerEnabled);
  Buzzer.setVolume(config.buzzerVolume);
  Buzzer.setTunes(config.tuneSuccess, config.tuneError, config.tuneBoot, config.tuneNfc);
}

bool LedBuzzerApp::playRing(const char* pattern) {
  if (!hackcardPeripheralsReady() || !pattern) {
    return false;
  }

  RgbRing.stopDemo();

  if (strcmp(pattern, "idle") == 0) {
    Status.setIdle();
    return true;
  }
  if (strcmp(pattern, "success") == 0) {
    Status.setSuccess();
    return true;
  }
  if (strcmp(pattern, "error") == 0) {
    Status.setError();
    return true;
  }
  if (strcmp(pattern, "scan") == 0) {
    Status.setScanning();
    return true;
  }
  if (strcmp(pattern, "boot") == 0) {
    Status.setBoot();
    return true;
  }
  if (strcmp(pattern, "pointer") == 0) {
    Status.setMenuPointer(0);
    return true;
  }

  return false;
}

bool LedBuzzerApp::playRingDemo(uint8_t patternId, uint8_t r, uint8_t g, uint8_t b) {
  if (!hackcardPeripheralsReady()) {
    return false;
  }

  if (patternId < 1 || patternId > 12) {
    return false;
  }

  RgbRing.startDemo(patternId, r, g, b);
  return true;
}

bool LedBuzzerApp::stopRingDemo() {
  if (!hackcardPeripheralsReady()) {
    return false;
  }

  RgbRing.stopDemo();
  Status.setIdle();
  return true;
}

bool LedBuzzerApp::playBuzzer(const char* tone) {
  if (!hackcardPeripheralsReady() || !tone) {
    return false;
  }

  if (strcmp(tone, "success") == 0) {
    Buzzer.playSuccess();
    return true;
  }
  if (strcmp(tone, "error") == 0) {
    Buzzer.playError();
    return true;
  }
  if (strcmp(tone, "boot") == 0) {
    Buzzer.playBoot();
    return true;
  }
  if (strcmp(tone, "nfc_ready") == 0) {
    Buzzer.playNfcReady();
    return true;
  }
  if (strcmp(tone, "nfc_write") == 0) {
    Buzzer.playNfcWriteOk();
    return true;
  }
  if (strcmp(tone, "nfc_clone") == 0) {
    Buzzer.playNfcCloneOk();
    return true;
  }
  if (strcmp(tone, "nfc_kill") == 0) {
    Buzzer.playNfcKillOk();
    return true;
  }

  return false;
}

bool LedBuzzerApp::playBuzzerTune(uint8_t tuneId) {
  if (!hackcardPeripheralsReady()) {
    return false;
  }

  if (tuneId >= static_cast<uint8_t>(BuzzerTuneId::Count)) {
    return false;
  }

  Buzzer.playTune(static_cast<BuzzerTuneId>(tuneId));
  return true;
}

bool LedBuzzerApp::playBuzzerDemo(uint8_t demoId) {
  if (!hackcardPeripheralsReady()) {
    return false;
  }

  return Buzzer.startSongMelodyAsync(demoId);
}

bool LedBuzzerApp::stopBuzzerMelody() {
  if (!hackcardPeripheralsReady()) {
    return false;
  }
  Buzzer.stopSongMelody();
  return true;
}

bool LedBuzzerApp::isBuzzerMelodyPlaying() const {
  return Buzzer.isSongMelodyPlaying();
}

bool LedBuzzerApp::setRingBrightness(uint8_t brightness, HackCardConfig& config) {
  if (brightness < 10) {
    brightness = 10;
  }
  if (brightness > 100) {
    brightness = 100;
  }

  config.ringBrightness = brightness;
  applySettings(config);
  return hackcardPeripheralsReady();
}

bool LedBuzzerApp::setBuzzerVolume(uint8_t volume, HackCardConfig& config) {
  if (volume < 10) {
    volume = 10;
  }
  if (volume > 100) {
    volume = 100;
  }

  config.buzzerVolume = volume;
  applySettings(config);
  return hackcardPeripheralsReady();
}

String buildVcard(const HackCardConfig& config) {
  String vcard;
  vcard += "BEGIN:VCARD\r\n";
  vcard += "VERSION:3.0\r\n";
  vcard += "FN:";
  vcard += config.ownerName;
  vcard += "\r\n";
  if (config.title[0] != '\0') {
    vcard += "TITLE:";
    vcard += config.title;
    vcard += "\r\n";
  }
  if (config.company[0] != '\0') {
    vcard += "ORG:";
    vcard += config.company;
    vcard += "\r\n";
  }
  if (config.email[0] != '\0') {
    vcard += "EMAIL:";
    vcard += config.email;
    vcard += "\r\n";
  }
  if (config.phone[0] != '\0') {
    vcard += "TEL:";
    vcard += config.phone;
    vcard += "\r\n";
  }
  if (config.website[0] != '\0') {
    vcard += "URL:";
    vcard += config.website;
    vcard += "\r\n";
  }
  if (config.linkedin[0] != '\0') {
    vcard += "X-SOCIALPROFILE;type=linkedin:";
    vcard += config.linkedin;
    vcard += "\r\n";
  }
  if (config.github[0] != '\0') {
    vcard += "X-SOCIALPROFILE;type=github:";
    vcard += config.github;
    vcard += "\r\n";
  }
  if (config.bio[0] != '\0') {
    vcard += "NOTE:";
    vcard += config.bio;
    vcard += "\r\n";
  }
  vcard += "END:VCARD\r\n";
  return vcard;
}

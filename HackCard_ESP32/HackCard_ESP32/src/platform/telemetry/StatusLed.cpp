#include "StatusLed.h"

StatusLed Status;

void StatusLed::setIdle() {
  RgbRing.setPattern(RingPattern::Idle);
}

void StatusLed::setBoot() {
  RgbRing.setPattern(RingPattern::BootAnim);
}

void StatusLed::setScanning() {
  RgbRing.applyWifiScanLook();
  WifiLed.setState(WifiLedState::Scanning);
}

void StatusLed::setSuccess() {
  RgbRing.setPattern(RingPattern::Success);
}

void StatusLed::setError() {
  RgbRing.setPattern(RingPattern::Error);
  WifiLed.setState(WifiLedState::Error);
}

void StatusLed::setStorageTier(bool sdReady) {
  RgbRing.setPattern(sdReady ? RingPattern::FullMode : RingPattern::FlashMode);
}

void StatusLed::setApActive(bool active) {
  WifiLed.setState(active ? WifiLedState::ApActive : WifiLedState::Off);
}

void StatusLed::setMenuPointer(uint8_t index) {
  RgbRing.setPointer(index);
}

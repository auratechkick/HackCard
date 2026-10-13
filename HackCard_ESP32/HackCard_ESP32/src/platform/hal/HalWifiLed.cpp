#include "HalWifiLed.h"

#include "config/board_config.h"

#include <Adafruit_NeoPixel.h>

namespace {

#if RGB_USE_GRB
constexpr neoPixelType kPixelType = NEO_GRB + NEO_KHZ800;
#else
constexpr neoPixelType kPixelType = NEO_RGB + NEO_KHZ800;
#endif

Adafruit_NeoPixel sPixel(RGB_WIFI_COUNT, PIN_RGB_WIFI, kPixelType);

}  // namespace

HalWifiLed WifiLed;

void HalWifiLed::begin(uint8_t brightness) {
  _brightness = brightness;
  sPixel.begin();
  sPixel.setBrightness(_brightness);
  sPixel.clear();
  sPixel.show();
}

void HalWifiLed::setBrightness(uint8_t brightness) {
  _brightness = brightness;
  sPixel.setBrightness(_brightness);
}

void HalWifiLed::setState(WifiLedState state) {
  _state = state;
}

void HalWifiLed::setTheme(uint8_t r, uint8_t g, uint8_t b, uint8_t pattern) {
  _themeR = r;
  _themeG = g;
  _themeB = b;
  _themePattern = pattern > 2 ? 2 : pattern;
}

uint32_t HalWifiLed::color(uint8_t r, uint8_t g, uint8_t b) const {
  return sPixel.Color(r, g, b);
}

void HalWifiLed::render() {
  uint32_t pixelColor = color(0, 0, 0);

  switch (_state) {
    case WifiLedState::Off:
      break;
    case WifiLedState::ApActive:
      if (_themePattern == 1) {
        pixelColor = color(_themeR, _themeG, _themeB);
      } else if (_themePattern == 2) {
        pixelColor = _pulseOn ? color(_themeR, _themeG, _themeB) : color(0, 0, 0);
      } else {
        pixelColor = _pulseOn ? color(_themeR, _themeG, _themeB)
                              : color(_themeR / 8, _themeG / 8, _themeB / 8);
      }
      break;
    case WifiLedState::StaConnected:
      pixelColor = color(0, 80, 255);
      break;
    case WifiLedState::Scanning:
      if (_themePattern == 1) {
        pixelColor = color(_themeR, _themeG, _themeB);
      } else if (_themePattern == 2) {
        pixelColor = _pulseOn ? color(_themeR, _themeG, _themeB) : color(0, 0, 0);
      } else {
        pixelColor = _pulseOn ? color(_themeR, _themeG, _themeB)
                              : color(_themeR / 8, _themeG / 8, _themeB / 8);
      }
      break;
    case WifiLedState::Error:
      pixelColor = _pulseOn ? color(220, 0, 0) : color(20, 0, 0);
      break;
    case WifiLedState::DataTransfer:
      pixelColor = _pulseOn ? color(0, 220, 80) : color(0, 40, 20);
      break;
  }

  sPixel.setPixelColor(0, pixelColor);
  sPixel.show();
}

void HalWifiLed::update() {
  const uint32_t now = millis();
  const uint32_t interval = _themePattern == 2 ? 120 : 300;
  if (now - _lastAnimMs >= interval) {
    _lastAnimMs = now;
    _pulseOn = !_pulseOn;
  }
  render();
}

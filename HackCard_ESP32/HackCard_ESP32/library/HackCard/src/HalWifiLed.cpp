#include "HalWifiLed.h"

#include "HackCard_Pins.h"

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

uint32_t HalWifiLed::color(uint8_t r, uint8_t g, uint8_t b) const {
  return sPixel.Color(r, g, b);
}

void HalWifiLed::render() {
  uint32_t pixelColor = color(0, 0, 0);

  switch (_state) {
    case WifiLedState::Off:
      break;
    case WifiLedState::ApActive:
      pixelColor = color(0, 180, 60);
      break;
    case WifiLedState::StaConnected:
      pixelColor = color(0, 80, 255);
      break;
    case WifiLedState::Scanning:
      pixelColor = _pulseOn ? color(0, 40, 255) : color(0, 8, 40);
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
  if (now - _lastAnimMs >= 300) {
    _lastAnimMs = now;
    _pulseOn = !_pulseOn;
  }
  render();
}

#include "HalRgbRing.h"

#include "HackCard_Pins.h"

#include <Adafruit_NeoPixel.h>
#include <cmath>

namespace {

#if RGB_USE_GRB
constexpr neoPixelType kPixelType = NEO_GRB + NEO_KHZ800;
#else
constexpr neoPixelType kPixelType = NEO_RGB + NEO_KHZ800;
#endif

Adafruit_NeoPixel sPixels(RGB_RING_COUNT, PIN_RGB_RING, kPixelType);

}  // namespace

HalRgbRing RgbRing;

void HalRgbRing::begin(uint8_t brightness) {
  _brightness = brightness;
  sPixels.begin();
  sPixels.setBrightness(_brightness);
  sPixels.clear();
  sPixels.show();
}

void HalRgbRing::setBrightness(uint8_t brightness) {
  _brightness = brightness;
  sPixels.setBrightness(_brightness);
}

void HalRgbRing::setPattern(RingPattern pattern) {
  if (_demoActive) {
    return;
  }
  _pattern = pattern;
  _animStep = 0;
  if (pattern != RingPattern::BootAnim) {
    _bootDone = true;
  }
}

void HalRgbRing::setPointer(uint8_t index) {
  if (_demoActive) {
    return;
  }
  _pointer = index % RGB_RING_COUNT;
  _pattern = RingPattern::Pointer;
}

void HalRgbRing::setProgress(uint8_t percent) {
  _progress = percent;
  if (_progress > 100) {
    _progress = 100;
  }
}

void HalRgbRing::showColor(uint8_t r, uint8_t g, uint8_t b) {
  if (_demoActive) {
    return;
  }
  for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
    sPixels.setPixelColor(i, scaleColor(r, g, b));
  }
  sPixels.show();
}

uint32_t HalRgbRing::scaleColor(uint8_t r, uint8_t g, uint8_t b) const {
  return sPixels.Color(r, g, b);
}

uint32_t HalRgbRing::scaleColorDim(uint8_t r, uint8_t g, uint8_t b, uint8_t scale) const {
  return scaleColor(static_cast<uint8_t>((r * scale) / 255), static_cast<uint8_t>((g * scale) / 255),
                    static_cast<uint8_t>((b * scale) / 255));
}

void HalRgbRing::startDemo(uint8_t patternId, uint8_t r, uint8_t g, uint8_t b) {
  _demoActive = true;
  _demoPattern = patternId < 1 ? 1 : (patternId > 12 ? 12 : patternId);
  _demoR = r;
  _demoG = g;
  _demoB = b;
  _animStep = 0;
}

void HalRgbRing::stopDemo() {
  _demoActive = false;
  _pattern = RingPattern::Idle;
  _animStep = 0;
}

uint32_t HalRgbRing::wheel(byte pos) const {
  pos = 255 - pos;
  if (pos < 85) {
    return scaleColor(255 - pos * 3, 0, pos * 3);
  }
  if (pos < 170) {
    pos -= 85;
    return scaleColor(0, pos * 3, 255 - pos * 3);
  }
  pos -= 170;
  return scaleColor(pos * 3, 255 - pos * 3, 0);
}

void HalRgbRing::renderDemo() {
  sPixels.clear();
  const uint8_t baseR = _demoR;
  const uint8_t baseG = _demoG;
  const uint8_t baseB = _demoB;
  const uint8_t step = _animStep;

  switch (_demoPattern) {
    case 1:  // Solid
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(baseR, baseG, baseB));
      }
      break;

    case 2: {  // Chase
      const uint8_t head = step % RGB_RING_COUNT;
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const bool lit = (i == head) || (i == ((head + RGB_RING_COUNT - 1) % RGB_RING_COUNT));
        sPixels.setPixelColor(i, lit ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 16));
      }
      break;
    }

    case 3:  // Rainbow
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const uint8_t hue = static_cast<uint8_t>(step + (i * 256 / RGB_RING_COUNT));
        sPixels.setPixelColor(i, wheel(hue));
      }
      break;

    case 4: {  // Breathe
      const uint8_t pulse = static_cast<uint8_t>((sin((step % 60) * 3.14159f / 30.0f) + 1.0f) * 127.5f);
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColorDim(baseR, baseG, baseB, pulse));
      }
      break;
    }

    case 5:  // Sparkle
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColorDim(baseR, baseG, baseB, 12));
      }
      for (uint8_t s = 0; s < 3; s++) {
        sPixels.setPixelColor(random(RGB_RING_COUNT), scaleColor(baseR, baseG, baseB));
      }
      break;

    case 6: {  // Wipe
      const uint8_t lit = (step / 2) % (RGB_RING_COUNT + 1);
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, i < lit ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 8));
      }
      break;
    }

    case 7:  // Alternate
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const bool lit = (i % 2) == (step % 2);
        sPixels.setPixelColor(i, lit ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 12));
      }
      break;

    case 8: {  // Dual chase
      const uint8_t head = step % RGB_RING_COUNT;
      const uint8_t tail = (head + RGB_RING_COUNT / 2) % RGB_RING_COUNT;
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const bool lit = (i == head) || (i == tail);
        sPixels.setPixelColor(i, lit ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 16));
      }
      break;
    }

    case 9: {  // Ping-pong
      const uint8_t cycle = (RGB_RING_COUNT - 1) * 2;
      const uint8_t pos = step % cycle;
      const uint8_t index = pos < (RGB_RING_COUNT - 1) ? pos : (cycle - pos);
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, i == index ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 12));
      }
      break;
    }

    case 10: {  // Theater
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const bool lit = ((i + step) % 3) == 0;
        sPixels.setPixelColor(i, lit ? scaleColor(baseR, baseG, baseB) : scaleColorDim(baseR, baseG, baseB, 10));
      }
      break;
    }

    case 11:  // Gradient
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const uint8_t fade = static_cast<uint8_t>(255 - (i * 220 / RGB_RING_COUNT));
        sPixels.setPixelColor(i, scaleColorDim(baseR, baseG, baseB, fade));
      }
      break;

    case 12:  // Strobe
    default: {
      const bool on = (step / 3) % 2 == 0;
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, on ? scaleColor(baseR, baseG, baseB) : 0);
      }
      break;
    }
  }
}

void HalRgbRing::render() {
  if (_demoActive) {
    renderDemo();
    sPixels.show();
    return;
  }

  sPixels.clear();

  switch (_pattern) {
    case RingPattern::Off:
      break;

    case RingPattern::BootAnim: {
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const int distance = abs(static_cast<int>(i) - static_cast<int>(_animStep % RGB_RING_COUNT));
        const uint8_t level = distance <= 1 ? 255 : 20;
        sPixels.setPixelColor(i, scaleColor(0, level / 3, level));
      }
      if (_animStep > RGB_RING_COUNT * 4) {
        _bootDone = true;
        _pattern = RingPattern::Idle;
      }
      break;
    }

    case RingPattern::Idle:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(0, 16, 32));
      }
      break;

    case RingPattern::Scanning: {
      const uint8_t head = _animStep % RGB_RING_COUNT;
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        const uint8_t level = (i == head) ? 255 : 20;
        sPixels.setPixelColor(i, scaleColor(0, 0, level));
      }
      break;
    }

    case RingPattern::Success:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(0, 180, 60));
      }
      break;

    case RingPattern::Error:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(180, 0, 0));
      }
      break;

    case RingPattern::Pointer:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        if (i == _pointer) {
          sPixels.setPixelColor(i, scaleColor(0, 120, 255));
        } else {
          sPixels.setPixelColor(i, scaleColor(0, 8, 20));
        }
      }
      break;

    case RingPattern::FullMode:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(0, 180, 60));
      }
      break;

    case RingPattern::FlashMode:
      for (uint8_t i = 0; i < RGB_RING_COUNT; i++) {
        sPixels.setPixelColor(i, scaleColor(180, 90, 0));
      }
      break;
  }

  if (_progress > 0 && _pattern != RingPattern::BootAnim) {
    const uint8_t lit = (RGB_RING_COUNT * _progress) / 100;
    for (uint8_t i = 0; i < lit && i < RGB_RING_COUNT; i++) {
      sPixels.setPixelColor(i, scaleColor(0, 180, 255));
    }
  }

  sPixels.show();
}

void HalRgbRing::update() {
  const uint32_t now = millis();
  if (now - _lastAnimMs < 60) {
    return;
  }
  _lastAnimMs = now;

  if (_pattern == RingPattern::BootAnim || _pattern == RingPattern::Scanning || _demoActive) {
    _animStep++;
  }

  render();
}

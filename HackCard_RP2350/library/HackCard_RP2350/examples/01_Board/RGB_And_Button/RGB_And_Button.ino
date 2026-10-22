/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: RGB strip and user button
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * Short press = next pattern. Long press = beep. Double press = off.
 */

#include <HackCardRP2350.h>

static const RingPattern kPatterns[] = {
  RingPattern::Idle,
  RingPattern::Scanning,
  RingPattern::Success,
  RingPattern::Error,
  RingPattern::FullMode,
  RingPattern::FlashMode
};
static const uint8_t kPatternCount = sizeof(kPatterns) / sizeof(kPatterns[0]);
static uint8_t gIndex = 0;

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — RGB and button");
  Serial.println("Short = next pattern, long = beep, double = off");

  RgbRing.begin(60);
  Buzzer.begin(true);
  Buttons.begin();
  RgbRing.setPattern(kPatterns[gIndex]);
}

void loop() {
  RgbRing.update();
  Buttons.update();

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      gIndex = (gIndex + 1) % kPatternCount;
      RgbRing.setPattern(kPatterns[gIndex]);
      Serial.printf("Pattern %u\n", gIndex);
      break;
    case ButtonGesture::Long:
      Buzzer.playTune(BuzzerTuneId::Chime);
      break;
    case ButtonGesture::Double:
      RgbRing.setPattern(RingPattern::Off);
      Serial.println("Off");
      break;
    default:
      break;
  }
}

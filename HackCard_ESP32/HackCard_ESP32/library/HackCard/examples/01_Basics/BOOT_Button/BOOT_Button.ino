/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: BOOT button gestures
 * FULL FIRMWARE: Physical menu / confirm on device
 *
 * WHAT IT DOES:
 * Detects short, long, and double press on BOOT (GPIO 0) with ring + buzzer feedback.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Press BOOT: short = pointer, long = success, double = error flash.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — BOOT Button");
  Serial.println("Short = next LED | Long = success beep | Double = error flash");

  Buttons.begin();
  RgbRing.begin(50);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static uint8_t pointer = 0;

  Buttons.update();
  RgbRing.update();

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      pointer = (pointer + 1) % RGB_RING_COUNT;
      RgbRing.setPointer(pointer);
      Serial.printf("Short press → pointer LED %u\n", pointer);
      break;

    case ButtonGesture::Long:
      Buzzer.playSuccess();
      RgbRing.setPattern(RingPattern::Success);
      Serial.println("Long press → success");
      break;

    case ButtonGesture::Double:
      Buzzer.playError();
      RgbRing.setPattern(RingPattern::Error);
      Serial.println("Double tap → error");
      break;

    default:
      break;
  }
}

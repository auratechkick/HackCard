/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Hello RGB ring
 * FULL FIRMWARE: Status ring patterns on Lab Console
 *
 * WHAT IT DOES:
 * Cycles Idle, Scanning, Success, Error, Full/Flash mode patterns on the 12-LED ring.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Watch the ring; pattern names print every 3 seconds.
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
  Serial.println("HackCard — Hello RGB Ring");
  Serial.println("Ring patterns will cycle every 3 seconds.");

  RgbRing.begin(60);
  RgbRing.setPattern(RingPattern::BootAnim);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static uint32_t lastSwitch = 0;
  static uint8_t step = 0;
  const RingPattern patterns[] = {
    RingPattern::Idle,
    RingPattern::Scanning,
    RingPattern::Success,
    RingPattern::Error,
    RingPattern::FullMode,
    RingPattern::FlashMode
  };
  const char* names[] = {
    "Idle", "Scanning", "Success", "Error", "FullMode", "FlashMode"
  };

  RgbRing.update();

  if (millis() - lastSwitch > 3000) {
    lastSwitch = millis();
    RgbRing.setPattern(patterns[step]);
    Serial.printf("Pattern: %s\n", names[step]);
    step = (step + 1) % 6;
  }
}

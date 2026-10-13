/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: LED ring demos 1-12
 * FULL FIRMWARE: /apps ring demo
 *
 * WHAT IT DOES:
 * Plays each built-in HalRgbRing demo pattern for 3 seconds.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Let the loop cycle; demos repeat forever.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — LED Ring Demos (web: /apps ring_demo)");
  RgbRing.begin(70);
  Buzzer.begin(true);
}

// loop() - keep calling Hal*.update() here
void loop() {
  for (uint8_t id = 1; id <= 12; id++) {
    Serial.printf("Demo pattern %u\n", id);
    RgbRing.startDemo(id, 0, 120, 255);
    uint32_t start = millis();
    while (millis() - start < 3500) {
      RgbRing.update();
      delay(10);
    }
    RgbRing.stopDemo();
  }
  Serial.println("--- repeat ---");
  Buzzer.playSuccess();
}

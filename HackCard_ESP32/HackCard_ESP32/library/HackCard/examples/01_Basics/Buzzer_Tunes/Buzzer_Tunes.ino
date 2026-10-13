/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Buzzer tunes
 * FULL FIRMWARE: /apps buzzer settings
 *
 * WHAT IT DOES:
 * Plays boot, success, error, and NFC-ready tunes on the PWM buzzer.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Listen and match tunes to HalBuzzer API calls in code.
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
  Serial.println("HackCard — Buzzer Tunes");
  Buzzer.begin(true);
}

// loop() - keep calling Hal*.update() here
void loop() {
  for (uint8_t i = 0; i < static_cast<uint8_t>(BuzzerTuneId::Count); i++) {
    Serial.printf("Playing: %s\n", HalBuzzer::tuneName(i));
    Buzzer.playTune(static_cast<BuzzerTuneId>(i));
    delay(1800);
  }

  Serial.println("--- repeat ---");
  delay(1000);
}

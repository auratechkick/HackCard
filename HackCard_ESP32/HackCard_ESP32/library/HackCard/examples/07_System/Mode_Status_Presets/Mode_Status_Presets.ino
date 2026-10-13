/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Mode status presets
 * FULL FIRMWARE: /modes
 *
 * WHAT IT DOES:
 * Cycles ring patterns used for Flash vs Full storage tier indicators.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Watch ring when switching modes on Serial.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

enum class Mode : uint8_t { Business, NfcLab, WifiAudit, DemoDay, Count };

static Mode gMode = Mode::Business;

void applyMode() {
  switch (gMode) {
    case Mode::Business:
      RgbRing.showColor(20, 80, 200);
      Serial.println("Mode: BUSINESS");
      break;
    case Mode::NfcLab:
      RgbRing.setPattern(RingPattern::Scanning);
      Serial.println("Mode: NFC_LAB");
      break;
    case Mode::WifiAudit:
      RgbRing.setPattern(RingPattern::Error);
      WifiLed.setState(WifiLedState::Scanning);
      Serial.println("Mode: WIFI_AUDIT");
      break;
    case Mode::DemoDay:
      RgbRing.setPattern(RingPattern::FullMode);
      Serial.println("Mode: DEMO_DAY");
      break;
    default:
      break;
  }
  Buzzer.playSuccess();
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Modes (web: /modes)");
  Serial.println("Short BOOT = next mode");
  Buttons.begin();
  RgbRing.begin(55);
  WifiLed.begin(50);
  Buzzer.begin(true);
  applyMode();
}

// loop() - keep calling Hal*.update() here
void loop() {
  Buttons.update();
  RgbRing.update();
  WifiLed.update();
  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    gMode = static_cast<Mode>((static_cast<uint8_t>(gMode) + 1) % static_cast<uint8_t>(Mode::Count));
    applyMode();
  }
}

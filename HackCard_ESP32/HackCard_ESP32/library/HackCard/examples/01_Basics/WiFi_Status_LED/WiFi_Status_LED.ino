/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi status LED
 * FULL FIRMWARE: Wi-Fi icon LED states on dashboard
 *
 * WHAT IT DOES:
 * Animates the single WS2812 near the Wi-Fi symbol through AP/STA/scan/error states.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Watch GPIO 38 LED and Serial state names.
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
  Serial.println("HackCard — Wi-Fi Status LED");
  WifiLed.begin(80);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static uint32_t last = 0;
  static uint8_t step = 0;
  const WifiLedState states[] = {
    WifiLedState::ApActive,
    WifiLedState::StaConnected,
    WifiLedState::Scanning,
    WifiLedState::DataTransfer,
    WifiLedState::Error,
    WifiLedState::Off
  };
  const char* names[] = {
    "AP active", "STA connected", "Scanning", "Data transfer", "Error", "Off"
  };

  WifiLed.update();

  if (millis() - last > 2500) {
    last = millis();
    WifiLed.setState(states[step]);
    Serial.printf("LED state: %s\n", names[step]);
    step = (step + 1) % 6;
  }
}

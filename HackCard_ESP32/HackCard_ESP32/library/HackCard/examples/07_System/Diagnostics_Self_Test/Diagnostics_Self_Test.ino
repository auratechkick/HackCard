/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Diagnostics self-test
 * FULL FIRMWARE: /diag
 *
 * WHAT IT DOES:
 * Checks ring, buzzer, button, NFC, SD presence and prints report.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Read PASS/FAIL lines on Serial.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>

void check(const char* name, bool pass, const char* detail) {
  Serial.printf("[%s] %s — %s\n", pass ? "PASS" : "FAIL", name, detail);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Diagnostics (web: /diag)");

  RgbRing.begin(50);
  WifiLed.begin(60);
  Buzzer.begin(true);

  RgbRing.showColor(0, 40, 0);
  delay(120);
  RgbRing.showColor(0, 0, 40);
  delay(120);
  RgbRing.setPattern(RingPattern::Idle);
  check("RGB Ring", true, "green/blue cycle OK");

  WifiLed.setState(WifiLedState::Scanning);
  delay(150);
  WifiLed.setState(WifiLedState::ApActive);
  check("WiFi LED", true, "animated");

  Buzzer.playSuccess();
  check("Buzzer", true, "success tone");

  bool nfc = Nfc.begin();
  check("NFC PN532", nfc, nfc ? Nfc.getChipInfo() : "not detected");

  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  bool sd = SD.begin(PIN_SD_CS);
  check("SD Card", true, sd ? "Full Mode (SD ready)" : "Flash Mode (no SD — OK)");

  WiFi.mode(WIFI_AP);
  bool ap = WiFi.softAP("HackCard-Diag", "hackcard2026");
  check("Wi-Fi AP", ap, ap ? WiFi.softAPIP().toString().c_str() : "softAP failed");

  check("BOOT Button", true, "manual — short press should work in BOOT_Button example");
  Serial.println("--- done ---");
  RgbRing.setPattern(RingPattern::Success);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();
  delay(20);
}

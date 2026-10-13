/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Evil twin simulation
 * FULL FIRMWARE: /wifi Evil twin
 *
 * WHAT IT DOES:
 * Training AP that mimics a named network for awareness labs.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit target SSID; authorized networks only.
 *
 * LIBRARIES: HackCard, WiFi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>

static const char* kLabPin = "1234";          // EDIT ME
static const char* kTargetSsid = "Cafe_WiFi"; // EDIT ME — lab SSID name only
static const char* kApPass = "hackcard2026";
static const uint8_t kDurationMin = 5;        // 1-15
static const char* kRestoreSsid = "HackCard-Setup";

static bool gRunning = false;
static uint32_t gEndsAt = 0;

void restoreAp() {
  WiFi.softAP(kRestoreSsid, kApPass);
  gRunning = false;
  Serial.printf("Restored AP: %s\n", kRestoreSsid);
  RgbRing.setPattern(RingPattern::Idle);
  WifiLed.setState(WifiLedState::ApActive);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Evil Twin Sim (web: /wifi twin)");
  Serial.println("Awareness demo only — authorized lab networks.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kRestoreSsid, kApPass);
  Serial.printf("Type PIN (%s) to start %u-min sim as \"%s\"\n",
                kLabPin, kDurationMin, kTargetSsid);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();

  if (!gRunning && Serial.available()) {
    String pin = Serial.readStringUntil('\n');
    pin.trim();
    if (pin != kLabPin) {
      Serial.println("PIN rejected.");
      Buzzer.playError();
      return;
    }
    if (!WiFi.softAP(kTargetSsid, kApPass)) {
      Serial.println("softAP failed");
      return;
    }
    gRunning = true;
    gEndsAt = millis() + (uint32_t)kDurationMin * 60UL * 1000UL;
    Serial.printf("Sim RUNNING as \"%s\" for %u min. Auto-restore after.\n",
                  kTargetSsid, kDurationMin);
    RgbRing.setPattern(RingPattern::Error);
    WifiLed.setState(WifiLedState::Scanning);
    Buzzer.playTune(BuzzerTuneId::Warning);
  }

  if (gRunning && (int32_t)(millis() - gEndsAt) >= 0) {
    Serial.println("Duration elapsed.");
    Buzzer.playSuccess();
    restoreAp();
  }
}

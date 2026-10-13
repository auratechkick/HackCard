/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi STA connect
 * FULL FIRMWARE: /wifi Connect
 *
 * WHAT IT DOES:
 * Joins your router as a Wi-Fi client (STA mode).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit SSID/password; upload; watch connect status.
 *
 * LIBRARIES: HackCard, WiFi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <WiFi.h>

static const char* kSsid = "YourHomeWiFi";  // EDIT ME
static const char* kPass = "password123";   // EDIT ME ("" for open)

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Wi-Fi Connect STA (web: /wifi connect)");
  RgbRing.begin(45);
  WifiLed.begin(70);
  WifiLed.setState(WifiLedState::Scanning);
  RgbRing.setPattern(RingPattern::Scanning);

  WiFi.mode(WIFI_STA);
  WiFi.begin(kSsid, kPass[0] ? kPass : nullptr);
  Serial.printf("Connecting to \"%s\"...\n", kSsid);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    WifiLed.update();
    RgbRing.update();
    delay(200);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Connected. IP=%s RSSI=%d\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
    WifiLed.setState(WifiLedState::StaConnected);
    RgbRing.setPattern(RingPattern::Success);
    Buzzer.begin(true);
    Buzzer.playSuccess();
  } else {
    Serial.println("Failed — check SSID/password (2.4 GHz only).");
    WifiLed.setState(WifiLedState::Error);
    RgbRing.setPattern(RingPattern::Error);
  }
}

// loop() - keep calling Hal*.update() here
void loop() {
  WifiLed.update();
  RgbRing.update();
  delay(20);
}

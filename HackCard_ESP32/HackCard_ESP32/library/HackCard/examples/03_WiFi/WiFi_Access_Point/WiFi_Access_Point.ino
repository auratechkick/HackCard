/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi soft AP
 * FULL FIRMWARE: /wifi AP settings
 *
 * WHAT IT DOES:
 * Starts HackCard as a 2.4 GHz access point (no web server in this sketch).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Join AP from phone; IP prints on Serial.
 *
 * LIBRARIES: HackCard, WiFi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <WiFi.h>

static const char* kSsid = "HackCard-Demo";
static const char* kPass = "hackcard2026";  // min 8 chars

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — Wi-Fi Access Point");

  WifiLed.begin(70);
  RgbRing.begin(40);
  WifiLed.setState(WifiLedState::Scanning);
  RgbRing.setPattern(RingPattern::Scanning);

  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(kSsid, kPass);
  if (!ok) {
    Serial.println("ERROR: softAP failed");
    WifiLed.setState(WifiLedState::Error);
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  IPAddress ip = WiFi.softAPIP();
  Serial.printf("AP SSID: %s\n", kSsid);
  Serial.printf("Password: %s\n", kPass);
  Serial.printf("AP IP: %s\n", ip.toString().c_str());
  Serial.println("Join from phone (2.4 GHz), then open the IP above.");

  WifiLed.setState(WifiLedState::ApActive);
  RgbRing.setPattern(RingPattern::Success);
}

// loop() - keep calling Hal*.update() here
void loop() {
  WifiLed.update();
  RgbRing.update();
  delay(20);
}

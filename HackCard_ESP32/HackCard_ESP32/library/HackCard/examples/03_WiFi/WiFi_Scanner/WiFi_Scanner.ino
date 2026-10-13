/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi scanner
 * FULL FIRMWARE: /wifi Scan
 *
 * WHAT IT DOES:
 * Scans 2.4 GHz networks and prints SSID/RSSI/channel.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Open Serial; scan runs periodically.
 *
 * LIBRARIES: HackCard, WiFi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <WiFi.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — Wi-Fi Scanner");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(200);

  RgbRing.begin(40);
  WifiLed.begin(70);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.setPattern(RingPattern::Scanning);
  WifiLed.setState(WifiLedState::Scanning);
  Serial.println();
  Serial.println("Scanning...");

  int n = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/true);
  RgbRing.update();
  WifiLed.update();

  if (n <= 0) {
    Serial.println("No networks found.");
    RgbRing.setPattern(RingPattern::Error);
    WifiLed.setState(WifiLedState::Error);
  } else {
    Serial.printf("Found %d networks:\n", n);
    for (int i = 0; i < n; i++) {
      Serial.printf("%2d  %s  RSSI=%d  ch=%d  %s\n",
                    i + 1,
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i),
                    WiFi.channel(i),
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "OPEN" : "SEC");
    }
    RgbRing.setPattern(RingPattern::Success);
    WifiLed.setState(WifiLedState::StaConnected);
  }

  WiFi.scanDelete();
  delay(5000);
}

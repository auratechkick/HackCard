/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Probe request sniff
 * FULL FIRMWARE: /wifi Monitor -> Probe sniff
 *
 * WHAT IT DOES:
 * Promiscuous capture of probe requests and SSIDs for 60s.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Type lab PIN on Serial; session runs once.
 *
 * LIBRARIES: HackCard, WiFi, esp_wifi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <esp_wifi.h>

static const char* kLabPin = "1234";
static const uint32_t kDurationMs = 60000;

static volatile bool gActive = false;

void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!gActive || type != WIFI_PKT_MGMT) return;
  const wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t* payload = pkt->payload;
  uint16_t len = pkt->rx_ctrl.sig_len;
  if (len < 24) return;
  uint8_t frameType = (payload[0] >> 2) & 0x03;
  uint8_t subtype = (payload[0] >> 4) & 0x0F;
  if (frameType != 0 || subtype != 4) return;  // probe request

  char ssid[33] = {0};
  uint8_t mac[6];
  memcpy(mac, payload + 10, 6);
  int pos = 24;
  while (pos + 2 <= (int)len) {
    uint8_t tag = payload[pos];
    uint8_t tagLen = payload[pos + 1];
    pos += 2;
    if (pos + tagLen > (int)len) break;
    if (tag == 0) {
      size_t n = tagLen > 32 ? 32 : tagLen;
      memcpy(ssid, payload + pos, n);
      ssid[n] = '\0';
      break;
    }
    pos += tagLen;
  }

  Serial.printf("probe RSSI=%d MAC=%02X:%02X:%02X:%02X:%02X:%02X SSID=\"%s\"\n",
                pkt->rx_ctrl.rssi,
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                ssid[0] ? ssid : "*");
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Probe Sniff (web: /wifi monitor sniff)");
  Serial.println("Authorized lab only.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);
  Serial.printf("Type PIN (%s) to start %lu ms sniff...\n", kLabPin, (unsigned long)kDurationMs);
}

// loop() - keep calling Hal*.update() here
void loop() {
  static bool done = false;
  RgbRing.update();
  WifiLed.update();
  if (done) return;

  if (!Serial.available()) return;
  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin != kLabPin) {
    Serial.println("PIN rejected.");
    Buzzer.playError();
    return;
  }

  WiFi.mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&sniffer);
  gActive = true;
  Serial.println("Sniffing probe requests...");
  RgbRing.setPattern(RingPattern::Scanning);
  WifiLed.setState(WifiLedState::Scanning);
  Buzzer.playNfcReady();

  uint32_t start = millis();
  while (millis() - start < kDurationMs) {
    RgbRing.update();
    WifiLed.update();
    delay(10);
  }

  gActive = false;
  esp_wifi_set_promiscuous(false);
  Serial.println("Sniff complete.");
  RgbRing.setPattern(RingPattern::Success);
  Buzzer.playSuccess();
  done = true;
}

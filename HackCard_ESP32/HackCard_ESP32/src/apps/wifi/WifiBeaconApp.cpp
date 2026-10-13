#include "WifiBeaconApp.h"

#include "config/app_registry.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/telemetry/StatusLed.h"
#if APP_WIFI_EVIL_TWIN
#include "WifiEvilTwinApp.h"
#endif
#if APP_WIFI_MONITOR
#include "WifiMonitorApp.h"
#endif

#include <cstring>
#include <esp_wifi.h>

WifiBeaconApp WifiBeacon;

namespace {

constexpr uint8_t kBeaconChannels[] = {1, 6, 11};

}  // namespace

void WifiBeaconApp::begin() {
  loadSsids();
}

void WifiBeaconApp::getApMac(uint8_t mac[6]) const {
  if (esp_wifi_get_mac(WIFI_IF_AP, mac) != ESP_OK) {
    memset(mac, 0, 6);
  }
}

bool WifiBeaconApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool WifiBeaconApp::parseSsidList(const char* text) {
  _ssidCount = 0;
  if (!text || text[0] == '\0') {
    return false;
  }

  String input = text;
  input.replace("\r", "\n");
  int start = 0;
  while (start < input.length() && _ssidCount < 8) {
    int end = input.indexOf('\n', start);
    if (end < 0) {
      end = input.length();
    }
    String line = input.substring(start, end);
    line.trim();
    if (!line.isEmpty() && line.length() <= 32) {
      strlcpy(_ssids[_ssidCount], line.c_str(), sizeof(_ssids[_ssidCount]));
      _ssidCount++;
    }
    start = end + 1;
  }
  return _ssidCount > 0;
}

bool WifiBeaconApp::saveSsids() const {
  Storage.ensureDirectory("/wifi");
  String text;
  for (uint8_t i = 0; i < _ssidCount; i++) {
    text += _ssids[i];
    text += "\n";
  }
  return Storage.writeTextFile("/wifi/beacon_ssids.txt", text);
}

bool WifiBeaconApp::loadSsids() {
  String text;
  if (!Storage.readTextFile("/wifi/beacon_ssids.txt", text)) {
    return false;
  }
  return parseSsidList(text.c_str());
}

String WifiBeaconApp::readSavedSsids() const {
  String text;
  if (Storage.readTextFile("/wifi/beacon_ssids.txt", text)) {
    return text;
  }

  if (_ssidCount > 0) {
    String built;
    for (uint8_t i = 0; i < _ssidCount; i++) {
      built += _ssids[i];
      built += "\n";
    }
    return built;
  }

  return "HackCard Demo\nGuest_WiFi\nFree_Internet\n";
}

int WifiBeaconApp::buildBeaconFrame(uint8_t* frame, const uint8_t* mac, const char* ssid) const {
  const size_t ssidLen = strlen(ssid);
  if (ssidLen == 0 || ssidLen > 32) {
    return 0;
  }

  int pos = 0;
  frame[pos++] = 0x80;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memset(&frame[pos], 0xff, 6);
  pos += 6;
  memcpy(&frame[pos], mac, 6);
  pos += 6;
  memcpy(&frame[pos], mac, 6);
  pos += 6;
  frame[pos++] = 0x00;
  frame[pos++] = 0x00;
  memset(&frame[pos], 0, 8);
  pos += 8;
  frame[pos++] = 0xE8;
  frame[pos++] = 0x03;
  frame[pos++] = 0x21;
  frame[pos++] = 0x04;
  frame[pos++] = 0x00;
  frame[pos++] = static_cast<uint8_t>(ssidLen);
  memcpy(&frame[pos], ssid, ssidLen);
  pos += ssidLen;
  frame[pos++] = 0x01;
  frame[pos++] = 0x08;
  const uint8_t rates[] = {0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c};
  memcpy(&frame[pos], rates, sizeof(rates));
  pos += sizeof(rates);
  return pos;
}

bool WifiBeaconApp::sendBeaconForSsid(const char* ssid, uint8_t channel, const uint8_t* mac) {
  uint8_t frame[256];
  const int len = buildBeaconFrame(frame, mac, ssid);
  if (len <= 0) {
    return false;
  }

  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  return esp_wifi_80211_tx(WIFI_IF_AP, frame, len, false) == ESP_OK;
}

WifiBeaconResult WifiBeaconApp::start(const char* labPin, const char* ssidListText) {
  WifiBeaconResult result;

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }

#if APP_WIFI_EVIL_TWIN
  if (WifiEvilTwin.isRunning()) {
    strncpy(result.error, "Stop evil-twin simulation first", sizeof(result.error) - 1);
    return result;
  }
#endif
#if APP_WIFI_MONITOR
  if (WifiMonitor.isSniffing() || WifiMonitor.isDeauthRunning()) {
    strncpy(result.error, "Stop Wi-Fi monitor lab first", sizeof(result.error) - 1);
    return result;
  }
#endif

  if (!WifiAp.verifyRunning()) {
    strncpy(result.error, "HackCard AP must be running", sizeof(result.error) - 1);
    return result;
  }

  if (!parseSsidList(ssidListText)) {
    strncpy(result.error, "Add 1-8 SSIDs (one per line)", sizeof(result.error) - 1);
    return result;
  }

  saveSsids();
  _packetsSent = 0;
  _ssidIndex = 0;
  _channelIndex = 0;
  _running = true;
  _lastSendMs = 0;

  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }

  Log.activity("wifi", "beacon_start", "lab");
  result.success = true;
  result.running = true;
  result.ssidCount = _ssidCount;
  result.packetsSent = _packetsSent;
  return result;
}

WifiBeaconResult WifiBeaconApp::stop() {
  WifiBeaconResult result;
  _running = false;
  Status.setApActive(true);
  Status.setIdle();

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }

  Log.activity("wifi", "beacon_stop", "ok");
  result.success = true;
  result.running = false;
  result.ssidCount = _ssidCount;
  result.packetsSent = _packetsSent;
  return result;
}

void WifiBeaconApp::loop() {
  if (!_running || _ssidCount == 0) {
    return;
  }

  const uint32_t now = millis();
  if (now - _lastSendMs < 120) {
    return;
  }
  _lastSendMs = now;

  uint8_t mac[6] = {0};
  getApMac(mac);

  const uint8_t channel = kBeaconChannels[_channelIndex % 3];
  if (sendBeaconForSsid(_ssids[_ssidIndex], channel, mac)) {
    _packetsSent++;
  }

  _ssidIndex = static_cast<uint8_t>((_ssidIndex + 1) % _ssidCount);
  if (_ssidIndex == 0) {
    _channelIndex = static_cast<uint8_t>((_channelIndex + 1) % 3);
  }
}

String WifiBeaconApp::statusToJson() const {
  String json = "{\"running\":";
  json += _running ? "true" : "false";
  json += ",\"ssid_count\":";
  json += String(_ssidCount);
  json += ",\"packets_sent\":";
  json += String(_packetsSent);
  json += ",\"ssids\":[";
  for (uint8_t i = 0; i < _ssidCount; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "\"";
    json += _ssids[i];
    json += "\"";
  }
  json += "]}";
  return json;
}

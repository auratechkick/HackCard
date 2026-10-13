#include "WifiCaptureApp.h"

#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
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
#if APP_WIFI_KARMA
#include "WifiKarmaApp.h"
#endif

#include <cstring>
#include <esp_wifi.h>

WifiCaptureApp WifiCapture;

namespace {

WifiCaptureApp* sCapture = nullptr;
constexpr uint8_t kMaxCaptureMin = 15;
constexpr uint8_t kQueueSize = 8;

struct CaptureQueueEntry {
  uint16_t len;
  int8_t rssi;
  uint8_t data[256];
};

CaptureQueueEntry sQueue[kQueueSize];
volatile uint8_t sHead = 0;
volatile uint8_t sTail = 0;

}  // namespace

void WifiCaptureApp::writePcapHeader() {
  uint8_t hdr[24] = {
      0xd4, 0xc3, 0xb2, 0xa1, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xff, 0xff, 0x00, 0x00, 0x69, 0x00, 0x00, 0x00,
  };
  if (_captureSdOnly) {
    Storage.writeBinaryFileSdOnly(_capturePath, hdr, sizeof(hdr));
  } else {
    Storage.writeBinaryFile(_capturePath, hdr, sizeof(hdr));
  }
}

void WifiCaptureApp::appendPcapRecord(const uint8_t* frame, uint16_t len, int8_t rssi) {
  const uint32_t ts = millis() * 1000U;
  uint8_t recHdr[16];
  recHdr[0] = ts & 0xFF;
  recHdr[1] = (ts >> 8) & 0xFF;
  recHdr[2] = (ts >> 16) & 0xFF;
  recHdr[3] = (ts >> 24) & 0xFF;
  recHdr[4] = recHdr[0];
  recHdr[5] = recHdr[1];
  recHdr[6] = recHdr[2];
  recHdr[7] = recHdr[3];
  recHdr[8] = len & 0xFF;
  recHdr[9] = (len >> 8) & 0xFF;
  recHdr[10] = len & 0xFF;
  recHdr[11] = (len >> 8) & 0xFF;
  (void)rssi;
  if (_captureSdOnly) {
    Storage.appendBinaryFileSdOnly(_capturePath, recHdr, sizeof(recHdr));
    Storage.appendBinaryFileSdOnly(_capturePath, frame, len);
  } else {
    Storage.appendBinaryFile(_capturePath, recHdr, sizeof(recHdr));
    Storage.appendBinaryFile(_capturePath, frame, len);
  }
}

void WifiCaptureApp::begin() {
  sCapture = this;
}

bool WifiCaptureApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool WifiCaptureApp::ensureLabIdle(char* error, size_t errorLen) const {
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
#if APP_WIFI_KARMA
  if (WifiKarma.isRunning()) {
    strncpy(error, "Stop karma session first", errorLen - 1);
    return false;
  }
#endif
  if (!WifiAp.verifyRunning()) {
    strncpy(error, "HackCard AP must be running", errorLen - 1);
    return false;
  }
  return true;
}

bool WifiCaptureApp::frameIsEapol(const uint8_t* payload, uint16_t len) const {
  if (!payload || len < 34) {
    return false;
  }
  const uint8_t frameType = (payload[0] >> 2) & 0x03;
  if (frameType != 2) {
    return false;
  }
  for (uint16_t i = 30; i + 2 < len; i++) {
    if (payload[i] == 0x88 && payload[i + 1] == 0x8E) {
      return true;
    }
  }
  return false;
}

void IRAM_ATTR WifiCaptureApp::onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!sCapture || !sCapture->_running || !buf) {
    return;
  }
  if (type != WIFI_PKT_DATA && type != WIFI_PKT_MGMT) {
    return;
  }
  const wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  const uint16_t len = pkt->rx_ctrl.sig_len;
  if (len == 0 || len > 256) {
    return;
  }
  const uint8_t next = static_cast<uint8_t>((sHead + 1) % kQueueSize);
  if (next == sTail) {
    return;
  }
  CaptureQueueEntry& e = sQueue[sHead];
  e.len = len;
  e.rssi = pkt->rx_ctrl.rssi;
  memcpy(e.data, pkt->payload, len);
  sHead = next;
}

void WifiCaptureApp::enablePromiscuous() {
  wifi_promiscuous_filter_t filter = {};
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA;
  esp_wifi_set_promiscuous_filter(&filter);
  esp_wifi_set_promiscuous_rx_cb(&WifiCaptureApp::onPromiscuous);
  esp_wifi_set_promiscuous(true);
}

void WifiCaptureApp::disablePromiscuous() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);
}

void WifiCaptureApp::appendFrame(const uint8_t* frame, uint16_t len, int8_t rssi) {
  appendPcapRecord(frame, len, rssi);
  _frameCount++;
  if (frameIsEapol(frame, len)) {
    _eapolCount++;
    _hasHandshakeHint = true;
    if (len > 80) {
      _hasPmkidHint = true;
    }
  }
}

void WifiCaptureApp::drainQueue() {
  while (sTail != sHead) {
    const CaptureQueueEntry& e = sQueue[sTail];
    appendFrame(e.data, e.len, e.rssi);
    sTail = static_cast<uint8_t>((sTail + 1) % kQueueSize);
  }
}

WifiCaptureResult WifiCaptureApp::start(const char* labPin, uint8_t channel, uint8_t durationMin, bool saveSdOnly) {
  WifiCaptureResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!ensureLabIdle(result.error, sizeof(result.error))) {
    return result;
  }
  if (durationMin == 0) {
    durationMin = 5;
  }
  if (durationMin > kMaxCaptureMin) {
    strncpy(result.error, "Capture max 15 minutes", sizeof(result.error) - 1);
    return result;
  }
  _captureSdOnly = saveSdOnly;
  if (_captureSdOnly) {
    if (!Storage.isSdReady()) {
      strncpy(result.error, "Insert an SD card for SD-only capture", sizeof(result.error) - 1);
      return result;
    }
    strncpy(_capturePath, "/wifi/exports/capture_live.pcap", sizeof(_capturePath) - 1);
    Storage.ensureDirectory("/wifi/exports");
  } else {
    strncpy(_capturePath, "/wifi/capture.pcap", sizeof(_capturePath) - 1);
    Storage.ensureDirectory("/wifi");
  }
  writePcapHeader();
  _channel = channel > 0 ? channel : static_cast<uint8_t>(WifiAp.getChannel());
  _frameCount = 0;
  _eapolCount = 0;
  _hasHandshakeHint = false;
  _hasPmkidHint = false;
  _running = true;
  _endsMs = millis() + static_cast<uint32_t>(durationMin) * 60U * 1000U;
  sHead = 0;
  sTail = 0;
  if (_channel > 0) {
    esp_wifi_set_channel(_channel, WIFI_SECOND_CHAN_NONE);
  }
  enablePromiscuous();
  Status.setScanning();
  Log.activity("wifi", "capture_start", "lab");
  result.success = true;
  return result;
}

WifiCaptureResult WifiCaptureApp::stop() {
  WifiCaptureResult result;
  if (!_running) {
    result.success = true;
    return result;
  }
  _running = false;
  disablePromiscuous();
  drainQueue();
  Status.setApActive(true);
  Status.setIdle();
  Log.activity("wifi", "capture_stop", "ok");
  result.success = true;
  return result;
}

void WifiCaptureApp::loop() {
  if (!_running) {
    return;
  }
  drainQueue();
  if (millis() >= _endsMs) {
    stop();
  }
}

bool WifiCaptureApp::hasCapture() const {
  return Storage.fileSize(_capturePath) > 24;
}

size_t WifiCaptureApp::pcapSize() const {
  return Storage.fileSize(_capturePath);
}

bool WifiCaptureApp::readPcapBytes(uint8_t* out, size_t maxLen, size_t& written) const {
  return Storage.readBinaryFile(_capturePath, out, maxLen, written);
}

WifiCaptureResult WifiCaptureApp::verifyPassword(const char* password, bool& matches) {
  WifiCaptureResult result;
  matches = false;
  if (!_hasHandshakeHint) {
    strncpy(result.error, "No handshake captured yet", sizeof(result.error) - 1);
    return result;
  }
  if (!password || password[0] == '\0') {
    strncpy(result.error, "Password required", sizeof(result.error) - 1);
    return result;
  }
  (void)password;
  strncpy(result.error, "On-device verify: capture OK — use exported .pcap with hashcat/aircrack-ng", sizeof(result.error) - 1);
  result.success = true;
  return result;
}

String WifiCaptureApp::statusToJson() const {
  String json = "{\"running\":";
  json += _running ? "true" : "false";
  json += ",\"channel\":";
  json += String(_channel);
  json += ",\"frames\":";
  json += String(_frameCount);
  json += ",\"eapol_frames\":";
  json += String(_eapolCount);
  json += ",\"has_handshake\":";
  json += _hasHandshakeHint ? "true" : "false";
  json += ",\"has_pmkid\":";
  json += _hasPmkidHint ? "true" : "false";
  json += ",\"pcap_bytes\":";
  json += String(pcapSize());
  json += ",\"ready\":";
  json += hasCapture() ? "true" : "false";
  json += ",\"has_capture\":";
  json += hasCapture() ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"capture_sd_only\":";
  json += _captureSdOnly ? "true" : "false";
  json += ",\"capture_path\":\"";
  json += _capturePath;
  json += "\"}";
  return json;
}

bool WifiCaptureApp::exportCaptureToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const {
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

  const size_t total = pcapSize();
  if (total <= 24) {
    if (error && errorLen > 0) {
      strncpy(error, "No capture file to export", errorLen - 1);
    }
    return false;
  }
  if (total > 512000) {
    if (error && errorLen > 0) {
      strncpy(error, "Capture too large — remove SD and copy live file", errorLen - 1);
    }
    return false;
  }

  uint8_t* buf = static_cast<uint8_t*>(malloc(total));
  if (!buf) {
    if (error && errorLen > 0) {
      strncpy(error, "Out of memory", errorLen - 1);
    }
    return false;
  }

  size_t readLen = 0;
  if (!readPcapBytes(buf, total, readLen) || readLen != total) {
    free(buf);
    if (error && errorLen > 0) {
      strncpy(error, "Could not read capture file", errorLen - 1);
    }
    return false;
  }

  Storage.ensureDirectory("/wifi/exports");
  snprintf(outPath, outPathLen, "/wifi/exports/capture_%lu.pcap", static_cast<unsigned long>(millis() / 1000UL));

  const bool ok = Storage.writeBinaryFileSdOnly(outPath, buf, readLen);
  free(buf);

  if (!ok) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not write export file on SD", errorLen - 1);
    }
    outPath[0] = '\0';
    return false;
  }

  Log.activity("wifi", "capture_export_sd", outPath);
  return true;
}

#include "WiFiHardwareTest.h"

#include "../../core/HcSerial.h"

#include "HalWifiAp.h"
#include "config/board_config.h"

#include <cstring>
#include <WiFi.h>
#include <esp_wifi.h>

WiFiHardwareTest WifiHwTest;

namespace {

int sLastDisconnectReason = 0;
uint8_t sTargetBssid[6] = {0};
bool sHasTargetBssid = false;
wifi_auth_mode_t sTargetAuthMode = WIFI_AUTH_OPEN;

struct ConnectStrategy {
  wifi_auth_mode_t authMode;
  bool pmfCapable;
  bool pmfRequired;
  bool useBssid;
  const char* label;
};

constexpr ConnectStrategy kConnectStrategies[] = {
    {WIFI_AUTH_WPA2_PSK, false, false, false, "WPA2-PSK, PMF off, no BSSID lock"},
    {WIFI_AUTH_WPA2_PSK, false, false, true, "WPA2-PSK, PMF off, BSSID lock"},
    {WIFI_AUTH_WPA2_PSK, true, false, false, "WPA2-PSK, PMF optional"},
    {WIFI_AUTH_WPA_PSK, true, false, false, "WPA/WPA2 mixed, PMF optional"},
    {WIFI_AUTH_WPA2_PSK, true, true, false, "WPA2-PSK, PMF required"},
    {WIFI_AUTH_WPA3_PSK, true, false, false, "WPA3 transition"},
};

const char* authModeName(wifi_auth_mode_t auth) {
  switch (auth) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK: return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
    default: return "?";
  }
}

wifi_auth_mode_t authFromLabel(const char* authLabel) {
  if (!authLabel || authLabel[0] == '\0') {
    return WIFI_AUTH_WPA2_PSK;
  }
  if (strcmp(authLabel, "OPEN") == 0) {
    return WIFI_AUTH_OPEN;
  }
  if (strcmp(authLabel, "WEP") == 0) {
    return WIFI_AUTH_WEP;
  }
  if (strcmp(authLabel, "WPA") == 0) {
    return WIFI_AUTH_WPA_PSK;
  }
  if (strcmp(authLabel, "WPA2") == 0 || strcmp(authLabel, "WPA/WPA2") == 0) {
    return WIFI_AUTH_WPA2_PSK;
  }
  if (strcmp(authLabel, "WPA3") == 0 || strcmp(authLabel, "WPA2/WPA3") == 0) {
    return WIFI_AUTH_WPA3_PSK;
  }
  if (strcmp(authLabel, "WPA2-ENT") == 0) {
    return WIFI_AUTH_WPA2_ENTERPRISE;
  }
  return WIFI_AUTH_WPA2_PSK;
}

bool parseBssidHex(const char* bssidText, uint8_t out[6]) {
  if (!bssidText || strlen(bssidText) < 17) {
    return false;
  }

  unsigned int bytes[6] = {0};
  if (sscanf(bssidText, "%02X:%02X:%02X:%02X:%02X:%02X", &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4],
             &bytes[5]) != 6) {
    return false;
  }

  for (int i = 0; i < 6; i++) {
    out[i] = static_cast<uint8_t>(bytes[i]);
  }
  return true;
}

void stepHeader(uint8_t step, uint8_t total, const char* title) {
  HcSerial.println();
  HcSerial.printf("[wifi-test] STEP %u/%u: %s\n", step, total, title);
}

void stepOk() {
  HcSerial.println("[wifi-test]   -> OK");
}

void stepFail(const char* reason) {
  HcSerial.printf("[wifi-test]   -> FAIL: %s\n", reason);
}

const char* statusName(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS: return "IDLE";
    case WL_NO_SSID_AVAIL: return "NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "SCAN_COMPLETED";
    case WL_CONNECTED: return "CONNECTED";
    case WL_CONNECT_FAILED: return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED: return "DISCONNECTED";
    default: return "UNKNOWN";
  }
}

const char* disconnectReasonText(int reason) {
  switch (reason) {
    case 2: return "AUTH_EXPIRE (password or WPA/PMF mismatch)";
    case 15: return "4WAY_HANDSHAKE_TIMEOUT (wrong password/WPA mode?)";
    case 201: return "NO_AP_FOUND";
    case 204: return "HANDSHAKE_TIMEOUT";
    case 205: return "CONNECTION_FAIL";
    default: return "see esp_wifi docs";
  }
}

void onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    sLastDisconnectReason = info.wifi_sta_disconnected.reason;
    HcSerial.printf("[wifi-test]   event: DISCONNECTED reason=%d (%s)\n", sLastDisconnectReason,
                  disconnectReasonText(sLastDisconnectReason));
  } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    HcSerial.println("[wifi-test]   event: GOT_IP");
  } else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
    HcSerial.println("[wifi-test]   event: CONNECTED to AP");
  }
}

bool initRadioSta() {
  HalWifiAp::preparePeripheralPins();

  WiFi.onEvent(onWifiEvent);
  WiFi.persistent(false);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.setAutoReconnect(false);
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(200);

  if (!WiFi.mode(WIFI_STA)) {
    return false;
  }

  delay(100);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  esp_wifi_set_ps(WIFI_PS_NONE);
  return true;
}

int findTargetChannel(const char* ssid) {
  const int count = WiFi.scanComplete();
  for (int i = 0; i < count; i++) {
    if (WiFi.SSID(i) == ssid) {
      return WiFi.channel(i);
    }
  }
  return 0;
}

bool startNativeConnect(const char* ssid, const char* password, const ConnectStrategy& strategy) {
  wifi_config_t conf = {};
  strlcpy(reinterpret_cast<char*>(conf.sta.ssid), ssid, sizeof(conf.sta.ssid));
  strlcpy(reinterpret_cast<char*>(conf.sta.password), password, sizeof(conf.sta.password));
  conf.sta.threshold.authmode = strategy.authMode;
  conf.sta.pmf_cfg.capable = strategy.pmfCapable;
  conf.sta.pmf_cfg.required = strategy.pmfRequired;

  if (strategy.useBssid && sHasTargetBssid) {
    conf.sta.bssid_set = true;
    memcpy(conf.sta.bssid, sTargetBssid, sizeof(sTargetBssid));
    HcSerial.printf("[wifi-test]   BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n", sTargetBssid[0], sTargetBssid[1],
                  sTargetBssid[2], sTargetBssid[3], sTargetBssid[4], sTargetBssid[5]);
  }

  esp_wifi_disconnect();
  delay(200);

  const esp_err_t cfgErr = esp_wifi_set_config(WIFI_IF_STA, &conf);
  if (cfgErr != ESP_OK) {
    HcSerial.printf("[wifi-test]   esp_wifi_set_config failed: %s\n", esp_err_to_name(cfgErr));
    return false;
  }

  const esp_err_t connErr = esp_wifi_connect();
  if (connErr != ESP_OK) {
    HcSerial.printf("[wifi-test]   esp_wifi_connect failed: %s\n", esp_err_to_name(connErr));
    return false;
  }
  return true;
}

bool waitForConnected(uint32_t timeoutMs);

bool tryArduinoBegin(const char* ssid, const char* password, int channel, bool useBssid, uint32_t timeoutMs,
                     WiFiTestResult& result) {
  HcSerial.printf("[wifi-test]   Trying: WiFi.begin() ch:%d bssid:%s\n", channel, useBssid ? "yes" : "no");
  sLastDisconnectReason = 0;
  WiFi.disconnect(false);
  delay(100);

  if (channel > 0 && useBssid && sHasTargetBssid) {
    WiFi.begin(ssid, password, channel, sTargetBssid);
  } else if (channel > 0) {
    WiFi.begin(ssid, password, channel);
  } else {
    WiFi.begin(ssid, password);
  }

  if (waitForConnected(timeoutMs)) {
    result.connectOk = true;
    result.ip = WiFi.localIP().toString();
    result.gateway = WiFi.gatewayIP().toString();
    HcSerial.printf("[wifi-test]   Connected using: WiFi.begin()\n");
    HcSerial.printf("[wifi-test]   IP:      %s\n", result.ip.c_str());
    HcSerial.printf("[wifi-test]   Gateway: %s\n", result.gateway.c_str());
    HcSerial.printf("[wifi-test]   RSSI:    %d dBm\n", WiFi.RSSI());
    stepOk();
    return true;
  }

  HcSerial.printf("[wifi-test]   WiFi.begin() failed, reason=%d\n", sLastDisconnectReason);
  return false;
}

bool waitForConnected(uint32_t timeoutMs) {
  const uint32_t start = millis();
  wl_status_t lastStatus = WL_IDLE_STATUS;
  int authFailCount = 0;

  while (millis() - start < timeoutMs) {
    const wl_status_t status = WiFi.status();
    if (status != lastStatus) {
      HcSerial.printf("[wifi-test]   status: %d (%s)\n", status, statusName(status));
      lastStatus = status;
    }

    if (status == WL_CONNECTED) {
      return true;
    }

    if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
      return false;
    }

    if (sLastDisconnectReason == 2 || sLastDisconnectReason == 15) {
      authFailCount++;
      if (authFailCount >= 2) {
        return false;
      }
    }

    delay(300);
  }
  return WiFi.status() == WL_CONNECTED;
}

WiFiTestResult attemptConnect(const char* ssid, const char* password, uint32_t timeoutMs, int channel,
                              bool reinitRadio, bool keepSoftAp) {
  WiFiTestResult result;

  if (reinitRadio && !keepSoftAp && !initRadioSta()) {
    result.failureReason = "Could not enter STA mode";
    stepFail(result.failureReason.c_str());
    return result;
  }
  if (reinitRadio && keepSoftAp && WiFi.getMode() != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
    delay(100);
  }

  sLastDisconnectReason = 0;

  HcSerial.printf("[wifi-test]   SSID: %s\n", ssid);
  HcSerial.printf("[wifi-test]   Password length: %u chars\n", strlen(password));
  HcSerial.printf("[wifi-test]   Timeout: %lu ms\n", timeoutMs);
  if (channel > 0) {
    HcSerial.printf("[wifi-test]   Channel: %d (from scan)\n", channel);
  }
  if (sTargetAuthMode != WIFI_AUTH_OPEN) {
    HcSerial.printf("[wifi-test]   Target auth: %s (%d)\n", authModeName(sTargetAuthMode), sTargetAuthMode);
  }

  WiFi.scanDelete();
  delay(100);
  // Keep the radio running — disconnect(true) calls esp_wifi_stop() and breaks esp_wifi_set_config.
  WiFi.disconnect(false);
  delay(100);
  if (keepSoftAp) {
    if (WiFi.getMode() != WIFI_AP_STA) {
      WiFi.mode(WIFI_AP_STA);
      delay(100);
    }
  } else if (WiFi.getMode() != WIFI_STA) {
    WiFi.mode(WIFI_STA);
    delay(100);
  }

  // Phone hotspots (auth:3 = WPA2): try simple Arduino connect first
  if (sTargetAuthMode == WIFI_AUTH_WPA2_PSK) {
    if (tryArduinoBegin(ssid, password, channel, /*useBssid=*/false, 10000, result)) {
      return result;
    }
  }

  // Mixed / enterprise routers: native ESP-IDF connect strategies
  for (const ConnectStrategy& strategy : kConnectStrategies) {
    sLastDisconnectReason = 0;
    HcSerial.printf("[wifi-test]   Trying: %s\n", strategy.label);
    if (!startNativeConnect(ssid, password, strategy)) {
      continue;
    }
    if (waitForConnected(8000)) {
      result.connectOk = true;
      result.ip = WiFi.localIP().toString();
      result.gateway = WiFi.gatewayIP().toString();
      HcSerial.printf("[wifi-test]   Connected using: %s\n", strategy.label);
      HcSerial.printf("[wifi-test]   IP:      %s\n", result.ip.c_str());
      HcSerial.printf("[wifi-test]   Gateway: %s\n", result.gateway.c_str());
      HcSerial.printf("[wifi-test]   RSSI:    %d dBm\n", WiFi.RSSI());
      stepOk();
      return result;
    }
    HcSerial.printf("[wifi-test]   Failed (%s), reason=%d\n", strategy.label, sLastDisconnectReason);
    esp_wifi_disconnect();
    delay(500);
  }

  // Final fallback: WiFi.begin with BSSID
  if (tryArduinoBegin(ssid, password, channel, /*useBssid=*/true, timeoutMs, result)) {
    return result;
  }

  String reason = "Timeout — last status=";
  reason += String(static_cast<int>(WiFi.status()));
  reason += " (";
  reason += statusName(WiFi.status());
  reason += ")";
  if (sLastDisconnectReason != 0) {
    reason += " disconnect=" + String(sLastDisconnectReason) + " (" + disconnectReasonText(sLastDisconnectReason) + ")";
  }
  result.failureReason = reason;
  stepFail(result.failureReason.c_str());
  return result;
}

bool fillScanResultFromCount(WifiScanResult& out, int count, uint32_t startMs, const char* highlightSsid, bool verbose) {
  if (count == WIFI_SCAN_FAILED) {
    strncpy(out.error, "WiFi.scanNetworks failed", sizeof(out.error) - 1);
    return false;
  }

  if (verbose) {
    HcSerial.printf("[wifi-test]   Found %d network(s)\n", count);
  }

  const int limit = count > 32 ? 32 : count;
  for (int i = 0; i < limit; i++) {
    WifiScanNetwork& net = out.networks[out.count];
    strlcpy(net.ssid, WiFi.SSID(i).c_str(), sizeof(net.ssid));
    net.rssi = static_cast<int8_t>(WiFi.RSSI(i));
    net.channel = static_cast<uint8_t>(WiFi.channel(i));
    strlcpy(net.auth, authModeName(WiFi.encryptionType(i)), sizeof(net.auth));

    const uint8_t* bssid = WiFi.BSSID(i);
    if (bssid) {
      snprintf(net.bssid, sizeof(net.bssid), "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3],
               bssid[4], bssid[5]);
    }

    if (verbose) {
      const char marker = (highlightSsid && strcmp(net.ssid, highlightSsid) == 0) ? '*' : ' ';
      HcSerial.printf("[wifi-test]   %c %2d. %-32s RSSI:%4d dBm  ch:%2d  auth:%s\n", marker, i + 1, net.ssid, net.rssi,
                    net.channel, net.auth);
    }

    if (highlightSsid && strcmp(net.ssid, highlightSsid) == 0 && bssid) {
      memcpy(sTargetBssid, bssid, sizeof(sTargetBssid));
      sHasTargetBssid = true;
      sTargetAuthMode = WiFi.encryptionType(i);
    }

    out.count++;
  }

  out.scanDurationMs = millis() - startMs;
  out.success = true;
  WiFi.scanDelete();
  return true;
}

}  // namespace

WiFiTestResult WiFiHardwareTest::scanNetworks(const char* highlightSsid) {
  WifiScanResult detailed;
  WiFiTestResult result;

  if (!scanNetworksDetailed(detailed, highlightSsid, /*verbose=*/true)) {
    result.failureReason = detailed.error;
    return result;
  }

  result.scanOk = true;
  result.networksFound = detailed.count;

  if (detailed.count == 0) {
    result.failureReason = "No networks seen — likely antenna/RF hardware issue";
    stepFail(result.failureReason.c_str());
    return result;
  }

  if (highlightSsid) {
    for (uint8_t i = 0; i < detailed.count; i++) {
      if (strcmp(detailed.networks[i].ssid, highlightSsid) == 0) {
        result.targetSeen = true;
        result.targetRssi = detailed.networks[i].rssi;
        result.targetChannel = detailed.networks[i].channel;
        break;
      }
    }
    if (!result.targetSeen) {
      HcSerial.printf("[wifi-test]   Target '%s' NOT seen in scan\n", highlightSsid);
    } else {
      HcSerial.printf("[wifi-test]   Target '%s' seen at %d dBm on channel %d\n", highlightSsid, result.targetRssi,
                    result.targetChannel);
    }
  }

  stepOk();
  return result;
}

bool WiFiHardwareTest::scanNetworksDetailed(WifiScanResult& out, const char* highlightSsid, bool verbose) {
  out = WifiScanResult();
  const uint32_t startMs = millis();

  if (verbose) {
    stepHeader(1, 1, "Scan for nearby 2.4 GHz networks");
  }

  sHasTargetBssid = false;
  sTargetAuthMode = WIFI_AUTH_OPEN;

  if (!initRadioSta()) {
    strncpy(out.error, "Could not enter STA mode for scan", sizeof(out.error) - 1);
    if (verbose) {
      stepFail(out.error);
    }
    return false;
  }

  if (verbose) {
    HcSerial.println("[wifi-test]   Scanning for 10 seconds...");
  }

  const int count = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/true, /*passive=*/false, /*max_ms=*/10000);

  if (!fillScanResultFromCount(out, count, startMs, highlightSsid, verbose)) {
    if (verbose) {
      stepFail(out.error);
    }
    return false;
  }

  return true;
}

bool WiFiHardwareTest::scanNetworksWithApUp(WifiScanResult& out, char* error, size_t errorLen) {
  if (error && errorLen > 0) {
    error[0] = '\0';
  }

  out = WifiScanResult();
  const uint32_t startMs = millis();

  if (!WifiAp.verifyRunning()) {
    if (error && errorLen > 0) {
      strncpy(error, "AP not running", errorLen - 1);
    }
    return false;
  }

  HalWifiAp::preparePeripheralPins();
  WiFi.persistent(false);
  WiFi.setSleep(WIFI_PS_NONE);

  if (WiFi.getMode() != WIFI_MODE_APSTA) {
    if (!WiFi.mode(WIFI_AP_STA)) {
      if (error && errorLen > 0) {
        strncpy(error, "Could not set AP+STA mode", errorLen - 1);
      }
      return false;
    }
    delay(80);
  }

  WiFi.scanDelete();
  sHasTargetBssid = false;
  sTargetAuthMode = WIFI_AUTH_OPEN;

  const int count = WiFi.scanNetworks(/*async=*/false, /*show_hidden=*/true, /*passive=*/false, /*max_ms=*/8000);
  if (!fillScanResultFromCount(out, count, startMs, nullptr, /*verbose=*/false)) {
    if (error && errorLen > 0) {
      strncpy(error, out.error, errorLen - 1);
    }
    return false;
  }

  return true;
}

bool WiFiHardwareTest::startScanAsync(bool keepApUp, char* error, size_t errorLen) {
  if (error && errorLen > 0) {
    error[0] = '\0';
  }

  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
    if (error && errorLen > 0) {
      strncpy(error, "Scan already running", errorLen - 1);
    }
    return false;
  }

  WiFi.scanDelete();
  sHasTargetBssid = false;
  sTargetAuthMode = WIFI_AUTH_OPEN;
  _asyncScanStartMs = millis();

  if (keepApUp && WifiAp.verifyRunning()) {
    HalWifiAp::preparePeripheralPins();
    WiFi.persistent(false);
    WiFi.setSleep(WIFI_PS_NONE);
    // Do not change WiFi.mode here — switching modes drops connected clients.
  } else if (!initRadioSta()) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not enter STA mode for scan", errorLen - 1);
    }
    return false;
  }

  const int started = WiFi.scanNetworks(/*async=*/true, /*show_hidden=*/true, /*passive=*/false, /*max_ms=*/10000);
  if (started == WIFI_SCAN_FAILED) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not start Wi-Fi scan", errorLen - 1);
    }
    return false;
  }

  return true;
}

bool WiFiHardwareTest::isScanRunning() const {
  return WiFi.scanComplete() == WIFI_SCAN_RUNNING;
}

bool WiFiHardwareTest::collectAsyncScanResults(WifiScanResult& out, char* error, size_t errorLen) {
  if (error && errorLen > 0) {
    error[0] = '\0';
  }

  out = WifiScanResult();
  const int count = WiFi.scanComplete();
  if (count == WIFI_SCAN_RUNNING) {
    if (error && errorLen > 0) {
      strncpy(error, "Scan still running", errorLen - 1);
    }
    return false;
  }

  if (!fillScanResultFromCount(out, count, _asyncScanStartMs, nullptr, /*verbose=*/false)) {
    if (error && errorLen > 0) {
      strncpy(error, out.error, errorLen - 1);
    }
    return false;
  }

  return true;
}

WiFiTestResult WiFiHardwareTest::connectToSta(const char* ssid, const char* password, uint32_t timeoutMs, int channel,
                                              bool keepSoftAp) {
  stepHeader(1, 1, "Connect to home router (STA mode)");
  if (!ssid) {
    WiFiTestResult result;
    result.failureReason = "Missing SSID";
    stepFail(result.failureReason.c_str());
    return result;
  }
  const char* pass = password ? password : "";
  if (!keepSoftAp && pass[0] == '\0') {
    WiFiTestResult result;
    result.failureReason = "Missing SSID or password";
    stepFail(result.failureReason.c_str());
    return result;
  }
  return attemptConnect(ssid, pass, timeoutMs, channel, /*reinitRadio=*/true, keepSoftAp);
}

WiFiTestResult WiFiHardwareTest::connectAfterScan(const char* ssid, const char* password, uint32_t timeoutMs,
                                                  int channel, bool keepSoftAp) {
  if (!ssid) {
    WiFiTestResult result;
    result.failureReason = "Missing SSID";
    stepFail(result.failureReason.c_str());
    return result;
  }
  const char* pass = password ? password : "";
  return attemptConnect(ssid, pass, timeoutMs, channel, /*reinitRadio=*/false, keepSoftAp);
}

void WiFiHardwareTest::disconnectSta() {
  WiFi.scanDelete();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_OFF);
  delay(200);
}

void WiFiHardwareTest::setConnectTarget(const char* authLabel, const char* bssid) {
  sTargetAuthMode = authFromLabel(authLabel);
  sHasTargetBssid = false;
  memset(sTargetBssid, 0, sizeof(sTargetBssid));

  if (bssid && parseBssidHex(bssid, sTargetBssid)) {
    sHasTargetBssid = true;
  }
}

WiFiTestResult WiFiHardwareTest::connectToNetwork(const char* ssid, const char* password, uint32_t timeoutMs,
                                                    uint8_t channel, const char* authLabel, const char* bssid,
                                                    bool scanFirstIfNeeded, bool keepSoftAp) {
  WiFiTestResult result;
  result.targetSsid = ssid ? ssid : "";

  if (!ssid || ssid[0] == '\0') {
    result.failureReason = "SSID required";
    return result;
  }

  const bool isOpen = authLabel && strcmp(authLabel, "OPEN") == 0;
  if (!isOpen && (!password || password[0] == '\0')) {
    result.failureReason = "Password required";
    return result;
  }

  setConnectTarget(authLabel, bssid);

  uint8_t useChannel = channel;
  bool scannedThisSession = false;

  if (useChannel == 0 && scanFirstIfNeeded) {
    WifiScanResult scan;
    if (!scanNetworksDetailed(scan, ssid, /*verbose=*/false)) {
      result.failureReason = scan.error[0] ? scan.error : "Scan failed";
      return result;
    }
    scannedThisSession = true;

    for (uint8_t i = 0; i < scan.count; i++) {
      if (strcmp(scan.networks[i].ssid, ssid) == 0) {
        useChannel = scan.networks[i].channel;
        setConnectTarget(scan.networks[i].auth, scan.networks[i].bssid);
        break;
      }
    }

    if (useChannel == 0) {
      result.failureReason = "Network not found in scan";
      return result;
    }
  }

  const char* pass = (isOpen || !password) ? "" : password;
  if (scannedThisSession) {
    result = connectAfterScan(ssid, pass, timeoutMs, useChannel, keepSoftAp);
  } else {
    result = connectToSta(ssid, pass, timeoutMs, useChannel, keepSoftAp);
  }

  result.targetSsid = ssid;
  return result;
}

WiFiTestResult WiFiHardwareTest::runFullTest(const char* targetSsid, const char* password, uint32_t connectTimeoutMs) {
  WiFiTestResult finalResult;
  finalResult.targetSsid = targetSsid ? targetSsid : "";

  HcSerial.println();
  HcSerial.println("╔══════════════════════════════════════════════╗");
  HcSerial.println("║     HackCard Wi-Fi Hardware Test             ║");
  HcSerial.println("╚══════════════════════════════════════════════╝");

  stepHeader(1, 6, "Prepare peripheral pins (high-Z)");
  HalWifiAp::preparePeripheralPins();
  stepOk();

  stepHeader(2, 6, "Initialize Wi-Fi radio");
  if (!initRadioSta()) {
    finalResult.failureReason = "Radio init failed";
    stepFail(finalResult.failureReason.c_str());
    printResult(finalResult);
    return finalResult;
  }
  finalResult.radioInitOk = true;
  HcSerial.printf("[wifi-test]   STA MAC: %s\n", WiFi.macAddress().c_str());
  stepOk();

  stepHeader(3, 6, "Scan for networks");
  WiFiTestResult scanResult = scanNetworks(targetSsid);
  finalResult.scanOk = scanResult.scanOk;
  finalResult.networksFound = scanResult.networksFound;
  finalResult.targetSeen = scanResult.targetSeen;
  finalResult.targetRssi = scanResult.targetRssi;
  finalResult.targetChannel = scanResult.targetChannel;
  if (!scanResult.scanOk) {
    finalResult.failureReason = scanResult.failureReason;
    printResult(finalResult);
    return finalResult;
  }

  stepHeader(4, 6, "Connect to target router (same session, no re-init)");
  WiFiTestResult connectResult =
      connectAfterScan(targetSsid, password, connectTimeoutMs, finalResult.targetChannel);
  finalResult.connectOk = connectResult.connectOk;
  finalResult.ip = connectResult.ip;
  finalResult.gateway = connectResult.gateway;
  if (!connectResult.connectOk) {
    finalResult.failureReason = connectResult.failureReason;
    printResult(finalResult);
    return finalResult;
  }

  stepHeader(5, 6, "Hold connection for 5 seconds");
  for (int i = 5; i > 0; i--) {
    HcSerial.printf("[wifi-test]   RSSI: %d dBm (%ds)\n", WiFi.RSSI(), i);
    delay(1000);
  }
  stepOk();

  stepHeader(6, 6, "Disconnect STA");
  disconnectSta();
  stepOk();

  printResult(finalResult);
  return finalResult;
}

void WiFiHardwareTest::printResult(const WiFiTestResult& result) const {
  HcSerial.println();
  HcSerial.println("[wifi-test] ========== VERDICT ==========");

  if (!result.radioInitOk) {
    HcSerial.println("[wifi-test] RADIO:     FAIL — Wi-Fi did not start");
  } else {
    HcSerial.println("[wifi-test] RADIO:     OK");
  }

  if (!result.scanOk || result.networksFound == 0) {
    HcSerial.println("[wifi-test] RX/SCAN:   FAIL — sees 0 networks (antenna/RF issue likely)");
  } else {
    HcSerial.printf("[wifi-test] RX/SCAN:   OK — saw %d network(s)\n", result.networksFound);
  }

  if (result.targetSeen) {
    HcSerial.printf("[wifi-test] TARGET:    OK — '%s' at %d dBm ch:%d\n", result.targetSsid.c_str(), result.targetRssi,
                  result.targetChannel);
  } else if (!result.targetSsid.isEmpty()) {
    HcSerial.printf("[wifi-test] TARGET:    FAIL — '%s' not seen\n", result.targetSsid.c_str());
  } else {
    HcSerial.println("[wifi-test] TARGET:    (not checked)");
  }

  if (result.connectOk) {
    HcSerial.printf("[wifi-test] TX/STA:    OK — connected, IP %s\n", result.ip.c_str());
    HcSerial.println("[wifi-test] HARDWARE:  Wi-Fi radio is working correctly");
  } else {
    HcSerial.println("[wifi-test] TX/STA:    FAIL — handshake rejected (reason=2)");
    HcSerial.println("[wifi-test]            RX works (scan OK). TX/auth may be failing.");
    HcSerial.println("[wifi-test]            Phone: Hotspot -> Security -> WPA2-Personal (not WPA3-only)");
    HcSerial.println("[wifi-test]            Phone: AP band -> 2.4 GHz");
    HcSerial.println("[wifi-test]            Serial: wifi connect 12345678");
    if (!result.failureReason.isEmpty()) {
      HcSerial.printf("[wifi-test] REASON:    %s\n", result.failureReason.c_str());
    }
  }

  HcSerial.println("[wifi-test] ==============================");
  HcSerial.println();
}

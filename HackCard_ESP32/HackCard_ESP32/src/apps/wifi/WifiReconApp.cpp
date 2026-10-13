#include "WifiReconApp.h"

#include "../../core/AppContext.h"
#include "../../core/LabPin.h"
#include "WifiStaStore.h"

extern AppContext gApp;
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/hal/WiFiHardwareTest.h"
#include "../../platform/telemetry/StatusLed.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <lwip/etharp.h>
#include <lwip/inet.h>
#include <lwip/netif.h>

WifiReconApp WifiRecon;

namespace {

constexpr uint16_t kTop20[] = {21, 22, 23, 25, 53, 80, 110, 135, 139, 143, 443, 445, 993, 995, 1723, 3306, 3389, 5900, 8080, 8443};
constexpr uint16_t kTop50Extra[] = {7, 9, 13, 17, 19, 37, 43, 67, 68, 69, 79, 88, 102, 111, 113, 119, 123, 137, 138, 161, 162, 179, 194, 389, 465, 514, 515, 631, 636, 873};

bool tcpProbe(const IPAddress& ip, uint16_t port, uint16_t timeoutMs) {
  WiFiClient client;
  client.setTimeout(timeoutMs);
  return client.connect(ip, port, timeoutMs);
}

bool arpProbe(const IPAddress& ip, uint8_t macOut[6]) {
  struct netif* netif = netif_default;
  if (!netif) {
    return false;
  }
  ip4_addr_t target = {};
  target.addr = static_cast<uint32_t>(ip);
  etharp_request(netif, &target);
  delay(12);
  struct eth_addr* ethRet = nullptr;
  const ip4_addr_t* ipRet = nullptr;
  if (etharp_find_addr(netif, &target, &ethRet, &ipRet) >= 0 && ethRet) {
    memcpy(macOut, ethRet->addr, 6);
    return true;
  }
  return false;
}

}  // namespace

bool WifiReconApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

WifiReconResult WifiReconApp::connectHold(const char* labPin, const char* ssid, const char* password) {
  WifiReconResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!ssid || ssid[0] == '\0') {
    strncpy(result.error, "SSID required", sizeof(result.error) - 1);
    return result;
  }

  const bool openNet = !password || password[0] == '\0';
  const char* authLabel = openNet ? "OPEN" : nullptr;
  const WiFiTestResult wr = WifiHwTest.connectToNetwork(ssid, password ? password : "", 25000, 0, authLabel, nullptr,
                                                        true, /*keepSoftAp=*/true);
  if (!wr.connectOk) {
    WifiAp.restart();
    strncpy(result.error, wr.failureReason.c_str(), sizeof(result.error) - 1);
    return result;
  }

  if (!WifiAp.ensureSoftApWithStaConnected()) {
    Log.warn("wifi", "STA hold OK but could not restore soft-AP");
  }
  Status.setApActive(WifiAp.verifyRunning());

  _staHeld = true;
  strlcpy(_staSsid, ssid, sizeof(_staSsid));
  strlcpy(_staIp, WiFi.localIP().toString().c_str(), sizeof(_staIp));
  WifiStaStoreSave(gApp, ssid, password ? password : "");
  Log.activity("wifi", "recon_sta_hold", ssid);
  result.success = true;
  return result;
}

WifiReconResult WifiReconApp::release() {
  WifiReconResult result;
  if (_staHeld) {
    WifiHwTest.disconnectSta();
    _staHeld = false;
    _staSsid[0] = '\0';
    _staIp[0] = '\0';
  }
  WifiAp.restart();
  Log.activity("wifi", "recon_sta_release", "ok");
  result.success = true;
  return result;
}

void WifiReconApp::prepareBootApOnly() {
  _staHeld = false;
  _staSsid[0] = '\0';
  _staIp[0] = '\0';
  _hostCount = 0;
  _openPortCount = 0;
  if (WiFi.status() == WL_CONNECTED) {
    WifiHwTest.disconnectSta();
  }
}

WifiReconResult WifiReconApp::subnetScan() {
  WifiReconResult result;
  if (!_staHeld || WiFi.status() != WL_CONNECTED) {
    strncpy(result.error, "Connect STA hold first", sizeof(result.error) - 1);
    return result;
  }

  const IPAddress ip = WiFi.localIP();
  const IPAddress mask = WiFi.subnetMask();
  const IPAddress network(ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], 0);
  _hostCount = 0;

  for (uint8_t host = 1; host < 255 && _hostCount < 32; host++) {
    IPAddress target(network[0], network[1], network[2], host);
    if (target == ip) {
      continue;
    }
    uint8_t mac[6] = {0};
    if (!arpProbe(target, mac)) {
      if (!tcpProbe(target, 80, 80) && !tcpProbe(target, 443, 80)) {
        continue;
      }
      snprintf(_hosts[_hostCount].mac, sizeof(_hosts[_hostCount].mac), "??:??:??:??:??:??");
    } else {
      snprintf(_hosts[_hostCount].mac, sizeof(_hosts[_hostCount].mac), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
               mac[2], mac[3], mac[4], mac[5]);
    }
    strlcpy(_hosts[_hostCount].ip, target.toString().c_str(), sizeof(_hosts[_hostCount].ip));
    if (target == WiFi.gatewayIP()) {
      strlcpy(_hosts[_hostCount].hostname, "gateway", sizeof(_hosts[_hostCount].hostname));
    } else if (host == 1) {
      strlcpy(_hosts[_hostCount].hostname, "router", sizeof(_hosts[_hostCount].hostname));
    } else {
      _hosts[_hostCount].hostname[0] = '\0';
    }
    _hosts[_hostCount].alive = true;
    _hostCount++;
  }

  Log.activity("wifi", "recon_subnet_scan", String(_hostCount).c_str());
  result.success = true;
  return result;
}

bool WifiReconApp::parsePortList(const char* csv, uint16_t* ports, uint8_t& count) const {
  count = 0;
  if (!csv || csv[0] == '\0') {
    return false;
  }
  if (strcmp(csv, "top20") == 0) {
    for (uint8_t i = 0; i < 20 && i < 64; i++) {
      ports[count++] = kTop20[i];
    }
    return true;
  }
  if (strcmp(csv, "top50") == 0) {
    for (uint8_t i = 0; i < 20; i++) {
      ports[count++] = kTop20[i];
    }
    for (uint8_t i = 0; i < 30 && count < 64; i++) {
      ports[count++] = kTop50Extra[i];
    }
    return true;
  }
  if (strcmp(csv, "iot") == 0) {
    const uint16_t iot[] = {80, 443, 1883, 5683, 8080, 8443, 554, 8554, 23, 502, 161};
    for (uint8_t i = 0; i < sizeof(iot) / sizeof(iot[0]) && count < 64; i++) {
      ports[count++] = iot[i];
    }
    return true;
  }

  String s = csv;
  int start = 0;
  while (start < s.length() && count < 64) {
    const int comma = s.indexOf(',', start);
    const String token = comma >= 0 ? s.substring(start, comma) : s.substring(start);
    const int port = token.toInt();
    if (port > 0 && port <= 65535) {
      ports[count++] = static_cast<uint16_t>(port);
    }
    if (comma < 0) {
      break;
    }
    start = comma + 1;
  }
  return count > 0;
}

WifiReconResult WifiReconApp::portScan(const char* targetIp, const char* portsCsv, uint16_t timeoutMs) {
  WifiReconResult result;
  if (!_staHeld || WiFi.status() != WL_CONNECTED) {
    strncpy(result.error, "Connect STA hold first", sizeof(result.error) - 1);
    return result;
  }
  if (!targetIp || targetIp[0] == '\0') {
    strncpy(result.error, "Target IP required", sizeof(result.error) - 1);
    return result;
  }

  IPAddress target;
  if (!target.fromString(targetIp)) {
    strncpy(result.error, "Invalid IP", sizeof(result.error) - 1);
    return result;
  }

  uint16_t ports[64];
  uint8_t portCount = 0;
  if (!parsePortList(portsCsv ? portsCsv : "top20", ports, portCount)) {
    strncpy(result.error, "No ports to scan", sizeof(result.error) - 1);
    return result;
  }
  if (timeoutMs == 0) {
    timeoutMs = 120;
  }

  strlcpy(_lastScanIp, targetIp, sizeof(_lastScanIp));
  _openPortCount = 0;
  for (uint8_t i = 0; i < portCount; i++) {
    const bool open = tcpProbe(target, ports[i], timeoutMs);
    if (open && _openPortCount < 64) {
      _openPorts[_openPortCount].port = ports[i];
      _openPorts[_openPortCount].open = true;
      _openPortCount++;
    }
  }

  Log.activity("wifi", "recon_port_scan", targetIp);
  result.success = true;
  return result;
}

String WifiReconApp::statusToJson() const {
  String json = "{\"sta_held\":";
  json += _staHeld ? "true" : "false";
  json += ",\"sta_ssid\":\"";
  json += _staSsid;
  json += "\",\"sta_ip\":\"";
  json += _staIp;
  if (_staHeld && _staIp[0] != '\0') {
    json += "\",\"lab_url\":\"http://";
    json += _staIp;
    json += "/lab\"";
  } else {
    json += "\",\"lab_url\":\"\"";
  }
  json += ",\"host_count\":";
  json += String(_hostCount);
  json += ",\"last_portscan_ip\":\"";
  json += _lastScanIp;
  json += "\"}";
  return json;
}

String WifiReconApp::hostsToJson() const {
  String json = "{\"count\":";
  json += String(_hostCount);
  json += ",\"hosts\":[";
  for (uint8_t i = 0; i < _hostCount; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"ip\":\"";
    json += _hosts[i].ip;
    json += "\",\"mac\":\"";
    json += _hosts[i].mac;
    json += "\",\"hostname\":\"";
    json += _hosts[i].hostname;
    json += "\"}";
  }
  json += "]}";
  return json;
}

String WifiReconApp::portScanToJson() const {
  String json = "{\"target\":\"";
  json += _lastScanIp;
  json += "\",\"open_ports\":[";
  bool first = true;
  for (uint8_t i = 0; i < _openPortCount; i++) {
    if (!_openPorts[i].open) {
      continue;
    }
    if (!first) {
      json += ",";
    }
    json += String(_openPorts[i].port);
    first = false;
  }
  json += "]}";
  return json;
}

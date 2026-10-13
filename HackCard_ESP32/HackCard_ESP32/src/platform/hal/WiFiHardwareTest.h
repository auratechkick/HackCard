#pragma once

#include <Arduino.h>

struct WiFiTestResult {
  bool radioInitOk = false;
  bool scanOk = false;
  int networksFound = 0;
  bool targetSeen = false;
  int targetRssi = 0;
  int targetChannel = 0;
  bool connectOk = false;
  String targetSsid;
  String ip;
  String gateway;
  String failureReason;
};

struct WifiScanNetwork {
  char ssid[33] = {0};
  int8_t rssi = 0;
  uint8_t channel = 0;
  char auth[16] = {0};
  char bssid[18] = {0};
};

struct WifiScanResult {
  bool success = false;
  uint8_t count = 0;
  WifiScanNetwork networks[32];
  char error[64] = {0};
  uint32_t scanDurationMs = 0;
};

class WiFiHardwareTest {
public:
  WiFiTestResult runFullTest(const char* targetSsid, const char* password, uint32_t connectTimeoutMs);
  WiFiTestResult scanNetworks(const char* highlightSsid = nullptr);
  bool scanNetworksDetailed(WifiScanResult& out, const char* highlightSsid = nullptr, bool verbose = false);
  bool scanNetworksWithApUp(WifiScanResult& out, char* error, size_t errorLen);
  bool startScanAsync(bool keepApUp, char* error, size_t errorLen);
  bool collectAsyncScanResults(WifiScanResult& out, char* error, size_t errorLen);
  bool isScanRunning() const;
  WiFiTestResult connectToSta(const char* ssid, const char* password, uint32_t timeoutMs, int channel = 0,
                              bool keepSoftAp = false);
  WiFiTestResult connectAfterScan(const char* ssid, const char* password, uint32_t timeoutMs, int channel,
                                  bool keepSoftAp = false);
  WiFiTestResult connectToNetwork(const char* ssid, const char* password, uint32_t timeoutMs, uint8_t channel,
                                  const char* authLabel, const char* bssid, bool scanFirstIfNeeded = true,
                                  bool keepSoftAp = false);
  void setConnectTarget(const char* authLabel, const char* bssid);
  void disconnectSta();
  void printResult(const WiFiTestResult& result) const;

private:
  uint32_t _asyncScanStartMs = 0;
};

extern WiFiHardwareTest WifiHwTest;

#pragma once

#include "../../platform/hal/WiFiHardwareTest.h"

struct WifiConnectResult {
  bool success = false;
  char ssid[33] = {0};
  char ip[16] = {0};
  char gateway[16] = {0};
  int8_t rssi = 0;
  uint8_t channel = 0;
  bool apRestored = false;
  char error[96] = {0};
};

class WifiConnectApp {
public:
  WifiConnectResult connectOnce(const char* ssid, const char* password, uint8_t channel, const char* authLabel,
                                const char* bssid);
  bool requestAsyncConnect(const char* ssid, const char* password, uint8_t channel, const char* authLabel,
                           const char* bssid);
  bool isConnecting() const { return _connectActive; }
  String toJson(const WifiConnectResult& result) const;
  String toText(const WifiConnectResult& result) const;
  String readLastConnectJson() const;
  String lastConnectApiJson() const;
  bool hasLastConnect() const;

private:
  static void connectWorkerEntry(void* param);
  void runConnectWorker();
  bool saveConnect(const WifiConnectResult& result);
  bool restoreAp();

  bool _connectActive = false;
  char _pendingSsid[33] = {0};
  char _pendingPassword[64] = {0};
  char _pendingAuth[16] = {0};
  char _pendingBssid[18] = {0};
  uint8_t _pendingChannel = 0;
  String _memoryCacheJson;
};

extern WifiConnectApp WifiConnect;

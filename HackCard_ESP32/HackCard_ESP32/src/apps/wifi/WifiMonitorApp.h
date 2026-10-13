#pragma once

#include <Arduino.h>
#include <esp_wifi.h>

struct WifiProbeEntry {
  uint32_t tsSec = 0;
  char ssid[33] = {0};
  char mac[18] = {0};
  int8_t rssi = 0;
  uint8_t channel = 0;
};

struct WifiApEntry {
  char ssid[33] = {0};
  char bssid[18] = {0};
  int8_t rssi = 0;
  uint8_t channel = 0;
  uint32_t beacons = 0;
};

struct WifiLiveEntry {
  uint32_t ms = 0;
  uint8_t channel = 0;
  int8_t rssi = 0;
  uint8_t type = 0;
  char summary[48] = {0};
};

struct WifiMonitorResult {
  bool success = false;
  char error[96] = {0};
};

class WifiMonitorApp {
 public:
  void begin();
  void loop();

  bool isSniffing() const { return _sniffing; }
  bool isDeauthRunning() const { return _deauthRunning; }

  WifiMonitorResult startSniff(const char* labPin, uint8_t durationMin, bool saveSdOnly = false);
  WifiMonitorResult stopSniff();
  bool exportProbeLogToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const;
  WifiMonitorResult startDeauth(const char* labPin, const char* bssidText, uint8_t channel, uint8_t frameCount,
                                uint8_t attackMode = 0);
  WifiMonitorResult stopDeauth();

  bool clearProbes();
  String probesToJson() const;
  String statusToJson() const;
  String apsToJson() const;
  String liveToJson() const;

 private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool ensureLabIdle(char* error, size_t errorLen) const;
  bool parseBssid(const char* text, uint8_t mac[6]) const;
  uint32_t getSniffRemainingSec() const;
  uint32_t getDeauthRemainingSec() const;

  void addProbe(const char* ssid, const uint8_t* mac, int8_t rssi, uint8_t channel);
  static void onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type);
  void drainProbeQueue();
  void drainBeaconQueue();
  void noteApBeacon(const char* ssid, const uint8_t* bssid, int8_t rssi, uint8_t channel);
  void pushLive(uint8_t type, uint8_t channel, int8_t rssi, const char* summary);
  void hopChannelIfNeeded();
  void enablePromiscuous();
  void disablePromiscuous();

  int buildDeauthFrame(uint8_t* frame, const uint8_t* bssid, uint16_t reason) const;
  int buildDisassocFrame(uint8_t* frame, const uint8_t* bssid, uint16_t reason) const;
  int buildAuthFloodFrame(uint8_t* frame, const uint8_t* bssid) const;
  int buildCsaFrame(uint8_t* frame, const uint8_t* bssid) const;
  int buildEapolBurstFrame(uint8_t* frame, const uint8_t* bssid) const;
  bool sendAttackFrame();

  bool _sniffing = false;
  bool _deauthRunning = false;
  bool _probeLogSdOnly = false;
  char _probeLogPath[48] = "/wifi/probe_log.txt";
  uint32_t _sniffEndsMs = 0;
  uint32_t _deauthEndsMs = 0;

  WifiProbeEntry _probes[40];
  uint8_t _probeHead = 0;
  uint8_t _probeCountBuf = 0;
  uint32_t _probeTotal = 0;

  uint32_t _packetTotal = 0;
  uint32_t _mgmtPackets = 0;
  uint32_t _dataPackets = 0;
  uint32_t _lastPacketMs = 0;

  WifiApEntry _aps[24];
  uint8_t _apCount = 0;

  WifiLiveEntry _live[16];
  uint8_t _liveCount = 0;
  uint8_t _liveHead = 0;

  uint8_t _hopChannel = 1;
  uint32_t _lastHopMs = 0;

  uint8_t _attackMode = 0;
  uint8_t _targetBssid[6] = {0};
  uint8_t _deauthChannel = 0;
  uint8_t _deauthLimit = 0;
  uint32_t _deauthSent = 0;
  uint32_t _lastDeauthMs = 0;
};

extern WifiMonitorApp WifiMonitor;

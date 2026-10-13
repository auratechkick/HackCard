#pragma once

#include <Arduino.h>

struct WifiReconResult {
  bool success = false;
  char error[96] = {0};
};

class WifiReconApp {
public:
  WifiReconResult connectHold(const char* labPin, const char* ssid, const char* password);
  WifiReconResult release();
  /** Drop any STA session before boot AP; does not erase saved SSID in settings. */
  void prepareBootApOnly();
  bool isStaHeld() const { return _staHeld; }

  WifiReconResult subnetScan();
  WifiReconResult portScan(const char* targetIp, const char* portsCsv, uint16_t timeoutMs);

  String statusToJson() const;
  String hostsToJson() const;
  String portScanToJson() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool parsePortList(const char* csv, uint16_t* ports, uint8_t& count) const;

  bool _staHeld = false;
  char _staSsid[33] = {0};
  char _staIp[16] = {0};
  char _lastScanIp[16] = {0};
  uint8_t _hostCount = 0;
  struct HostEntry {
    char ip[16];
    char mac[18];
    char hostname[32];
    bool alive;
  };
  HostEntry _hosts[32];
  uint8_t _openPortCount = 0;
  struct PortEntry {
    uint16_t port;
    bool open;
  };
  PortEntry _openPorts[64];
};

extern WifiReconApp WifiRecon;

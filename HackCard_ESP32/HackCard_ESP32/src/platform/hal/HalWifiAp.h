#pragma once

#include <Arduino.h>
#include <WiFi.h>

class HalWifiAp {
public:
  bool begin(const char* ssid, const char* password, int channel = 0);
  void maintain();
  bool verifyRunning() const;
  bool restart();
  /** Start or restore soft-AP without disconnecting an active STA (AP+STA mode). */
  bool ensureSoftApWithStaConnected();

  static void preparePeripheralPins();

  bool isRunning() const { return _running; }
  int getChannel() const { return _channel; }

  String getStatusText() const;
  String getSsid() const { return _ssid; }
  String getIp() const;
  String getLastError() const { return _lastError; }

private:
  void preparePinsForWifi();
  bool startAp(const char* ssid, const char* password, int channel);
  bool tryChannels(const char* ssid, const char* password, int preferredChannel);

  bool _running = false;
  String _ssid;
  String _password;
  String _lastError;
  int _channel = 1;
  uint32_t _lastMaintainMs = 0;
};

extern HalWifiAp WifiAp;

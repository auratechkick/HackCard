#pragma once

#include <Arduino.h>

struct WifiBeaconResult {
  bool success = false;
  bool running = false;
  uint8_t ssidCount = 0;
  uint32_t packetsSent = 0;
  char error[64] = {0};
};

class WifiBeaconApp {
public:
  void begin();
  void loop();

  WifiBeaconResult start(const char* labPin, const char* ssidListText);
  WifiBeaconResult stop();
  bool isRunning() const { return _running; }
  uint8_t getSsidCount() const { return _ssidCount; }
  uint32_t getPacketsSent() const { return _packetsSent; }

  String statusToJson() const;
  String readSavedSsids() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool parseSsidList(const char* text);
  bool saveSsids() const;
  bool loadSsids();
  int buildBeaconFrame(uint8_t* frame, const uint8_t* mac, const char* ssid) const;
  bool sendBeaconForSsid(const char* ssid, uint8_t channel, const uint8_t* mac);
  void getApMac(uint8_t mac[6]) const;

  bool _running = false;
  uint8_t _ssidCount = 0;
  char _ssids[8][33] = {{0}};
  uint32_t _packetsSent = 0;
  uint32_t _lastSendMs = 0;
  uint8_t _ssidIndex = 0;
  uint8_t _channelIndex = 0;
};

extern WifiBeaconApp WifiBeacon;

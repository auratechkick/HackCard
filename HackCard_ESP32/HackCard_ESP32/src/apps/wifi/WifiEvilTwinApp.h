#pragma once

#include "../../core/AppContext.h"

#include <Arduino.h>

struct WifiEvilTwinResult {
  bool success = false;
  bool running = false;
  uint32_t remainingSec = 0;
  char error[96] = {0};
};

class WifiEvilTwinApp {
public:
  WifiEvilTwinResult start(AppContext& ctx, const char* labPin, const char* targetSsid, uint8_t channel,
                           uint8_t durationMin);
  WifiEvilTwinResult stop(AppContext& ctx);
  void loop(AppContext& ctx);

  bool isRunning() const { return _running; }
  const char* getTargetSsid() const { return _targetSsid; }
  uint32_t getRemainingSec() const;
  String statusToJson() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool validateTarget(const char* targetSsid, uint8_t channel, uint8_t durationMin, char* error,
                      size_t errorLen) const;
  void restoreAp(AppContext& ctx);

  bool _running = false;
  char _targetSsid[32] = {0};
  uint8_t _channel = 0;
  uint32_t _endsAtMs = 0;
  uint32_t _durationSec = 0;
  char _restoreSsid[32] = {0};
  char _restorePassword[64] = {0};
  uint8_t _restoreChannel = 0;
  bool _restorePortalEnabled = false;
};

extern WifiEvilTwinApp WifiEvilTwin;

#pragma once

#include "../../core/AppContext.h"

#include <Arduino.h>

struct BleContactCardResult {
  bool success = false;
  bool active = false;
  char error[64] = {0};
};

class BleContactCardApp {
public:
  BleContactCardResult start(const HackCardConfig& config);
  BleContactCardResult stop();
  BleContactCardResult refresh(const HackCardConfig& config);

  bool isActive() const { return _active; }
  const char* getContactName() const { return _contactName; }
  String statusToJson() const;

private:
  bool setupService(const HackCardConfig& config);
  bool startAdvertising(const HackCardConfig& config);
  void stopAdvertising();

  bool _active = false;
  bool _serviceReady = false;
  char _contactName[64] = {0};
};

extern BleContactCardApp BleContactCard;

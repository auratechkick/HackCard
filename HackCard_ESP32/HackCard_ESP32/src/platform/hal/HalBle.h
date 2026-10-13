#pragma once

#include <Arduino.h>

class HalBle {
public:
  bool ensureInit();
  bool isReady() const { return _ready; }
  bool isAdvertising() const { return _advertising; }

  bool startAdvertising(const char* name);
  void stopAdvertising();

private:
  bool _ready = false;
  bool _advertising = false;
};

extern HalBle Ble;

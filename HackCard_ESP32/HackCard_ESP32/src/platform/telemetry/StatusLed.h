#pragma once

#include "../hal/HalRgbRing.h"
#include "../hal/HalWifiLed.h"

class StatusLed {
public:
  void setIdle();
  void setBoot();
  void setScanning();
  void setSuccess();
  void setError();
  void setStorageTier(bool sdReady);
  void setApActive(bool active);
  void setMenuPointer(uint8_t index);
};

extern StatusLed Status;

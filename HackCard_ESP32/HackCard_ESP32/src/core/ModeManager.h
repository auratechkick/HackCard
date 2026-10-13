#pragma once

#include "DeviceModeManager.h"

class ModeManager {
 public:
  static const char* toString(DeviceMode mode) { return DeviceModeManager::toString(mode); }
  static DeviceMode fromString(const char* name) { return DeviceModeManager::fromString(name); }
  static void apply(DeviceMode mode);
  static const char* description(DeviceMode mode) { return DeviceModeManager::spec(mode).description; }
};

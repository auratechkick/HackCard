#pragma once

#include "AppContext.h"

#include <Arduino.h>

enum class DeviceMode : uint8_t {
  Business = 0,
  Lab = 1,
};

struct DeviceModeSpec {
  const char* id;
  const char* title;
  const char* description;
  bool showBusinessProfile;
  bool captivePortal;
  const char* ledProfile;
};

class DeviceModeManager {
 public:
  static const char* toString(DeviceMode mode);
  static DeviceMode fromString(const char* name);
  static DeviceMode fromStored(uint8_t raw);
  static DeviceModeSpec spec(DeviceMode mode);
  static uint8_t count();
  static const DeviceModeSpec* specAt(uint8_t index);

  static bool isBusinessMode(DeviceMode mode);
  static bool isBusinessMode(const AppContext& ctx);
  static bool isLabMode(DeviceMode mode);
  static bool isLabMode(const AppContext& ctx);

  static void apply(AppContext& ctx, DeviceMode mode);
  static void applyFromConfig(AppContext& ctx);
  static String listToJson(DeviceMode current);
};

#include "DeviceModeManager.h"

#include "BootState.h"
#include "Logger.h"
#include "../platform/telemetry/StatusLed.h"
#if APP_BLE
#include "../apps/ble/BleContactCardApp.h"
#endif

namespace {

struct ModeEntry {
  DeviceMode mode;
  DeviceModeSpec spec;
};

ModeEntry kModes[] = {
    {DeviceMode::Lab,
     {"lab", "Lab Mode", "Full HackCard dashboard and tools — configure profile here", false, false, "scan"}},
    {DeviceMode::Business,
     {"business", "Business Mode", "Professional contact profile for guests — lab tools require password",
      true, true, "idle"}},
};

DeviceModeSpec specFor(DeviceMode mode) {
  for (const ModeEntry& entry : kModes) {
    if (entry.mode == mode) {
      return entry.spec;
    }
  }
  return kModes[0].spec;
}

}  // namespace

const char* DeviceModeManager::toString(DeviceMode mode) {
  return specFor(mode).id;
}

DeviceMode DeviceModeManager::fromString(const char* name) {
  if (!name || name[0] == '\0') {
    return DeviceMode::Lab;
  }
  for (const ModeEntry& entry : kModes) {
    if (strcmp(name, entry.spec.id) == 0) {
      return entry.mode;
    }
  }
  if (strcmp(name, "business_card") == 0 || strcmp(name, "conference") == 0 ||
      strcmp(name, "consultant") == 0 || strcmp(name, "demo_day") == 0 || strcmp(name, "demo") == 0) {
    return DeviceMode::Business;
  }
  if (strcmp(name, "lab_only") == 0 || strcmp(name, "nfc_lab") == 0 || strcmp(name, "wifi_audit") == 0 ||
      strcmp(name, "nfc") == 0 || strcmp(name, "wifi") == 0) {
    return DeviceMode::Lab;
  }
  return DeviceMode::Lab;
}

DeviceMode DeviceModeManager::fromStored(uint8_t raw) {
  if (raw == static_cast<uint8_t>(DeviceMode::Lab)) {
    return DeviceMode::Lab;
  }
  return DeviceMode::Business;
}

DeviceModeSpec DeviceModeManager::spec(DeviceMode mode) {
  return specFor(mode);
}

uint8_t DeviceModeManager::count() {
  return sizeof(kModes) / sizeof(kModes[0]);
}

const DeviceModeSpec* DeviceModeManager::specAt(uint8_t index) {
  if (index >= count()) {
    return nullptr;
  }
  return &kModes[index].spec;
}

bool DeviceModeManager::isBusinessMode(DeviceMode mode) {
  return mode == DeviceMode::Business;
}

bool DeviceModeManager::isBusinessMode(const AppContext& ctx) {
  return isBusinessMode(fromStored(ctx.config.deviceMode));
}

bool DeviceModeManager::isLabMode(DeviceMode mode) {
  return mode == DeviceMode::Lab;
}

bool DeviceModeManager::isLabMode(const AppContext& ctx) {
  return isLabMode(fromStored(ctx.config.deviceMode));
}

void DeviceModeManager::apply(AppContext& ctx, DeviceMode mode) {
  const DeviceModeSpec ms = specFor(mode);
  ctx.config.deviceMode = static_cast<uint8_t>(mode);
  ctx.config.captivePortalEnabled = ms.captivePortal;

  if (hackcardPeripheralsReady()) {
    if (strcmp(ms.ledProfile, "scan") == 0) {
      Status.setScanning();
    } else {
      Status.setIdle();
    }
  }

#if APP_BLE
  if (BleContactCard.isActive()) {
    BleContactCard.stop();
  }
#else
  (void)ctx;
#endif

  Log.activity("mode", "apply", ms.id);
}

void DeviceModeManager::applyFromConfig(AppContext& ctx) {
  apply(ctx, fromStored(ctx.config.deviceMode));
}

String DeviceModeManager::listToJson(DeviceMode current) {
  String json = "{\"current\":\"";
  json += toString(current);
  json += "\",\"modes\":[";
  for (uint8_t i = 0; i < count(); i++) {
    const DeviceModeSpec* ms = specAt(i);
    if (!ms) {
      continue;
    }
    if (i > 0) {
      json += ",";
    }
    json += "{\"id\":\"";
    json += ms->id;
    json += "\",\"title\":\"";
    json += ms->title;
    json += "\",\"description\":\"";
    json += ms->description;
    json += "\",\"business_profile\":";
    json += ms->showBusinessProfile ? "true" : "false";
    json += "}";
  }
  json += "]}";
  return json;
}

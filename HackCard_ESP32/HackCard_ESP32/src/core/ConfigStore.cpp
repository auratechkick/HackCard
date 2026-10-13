#include "ConfigStore.h"

#include "DeviceModeManager.h"
#include "HcSerial.h"
#include "StorageManager.h"

#include <ArduinoJson.h>
#include <Preferences.h>

ConfigStore Config;

namespace {

constexpr const char* kConfigPath = "/config/hackcard.json";
constexpr const char* kNvsNamespace = "hackcard";
constexpr const char* kNvsConfigKey = "json";
constexpr size_t kConfigJsonCapacity = 4096;

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 7
using ConfigJsonDocument = JsonDocument;
#else
using ConfigJsonDocument = StaticJsonDocument<kConfigJsonCapacity>;
#endif

}  // namespace

bool ConfigStore::begin(AppContext& ctx) {
  return load(ctx);
}

bool ConfigStore::load(AppContext& ctx) {
  String json;
  if (Storage.readTextFile(kConfigPath, json) && loadFromJson(ctx, json)) {
    if (Storage.isSdReady()) {
      strncpy(_sourceLabel, "sd", sizeof(_sourceLabel) - 1);
    } else {
      strncpy(_sourceLabel, "flash", sizeof(_sourceLabel) - 1);
    }
    saveJsonToNvs(json);
    return true;
  }

  if (loadJsonFromNvs(json) && loadFromJson(ctx, json)) {
    strncpy(_sourceLabel, "nvs", sizeof(_sourceLabel) - 1);
    HcSerial.println("[config] Loaded from NVS backup (flash file missing or invalid)");
    Storage.writeTextFile(kConfigPath, json);
    return true;
  }

  loadFromCompiledDefaults(ctx);
  strncpy(_sourceLabel, "compiled", sizeof(_sourceLabel) - 1);
  HcSerial.println("[config] Using compiled defaults — save profile/setup to persist on device");
  return true;
}

bool ConfigStore::loadFromCompiledDefaults(AppContext& ctx) {
  ctx.applyUserConfigDefaults();
  return true;
}

bool ConfigStore::loadFromJson(AppContext& ctx, const String& json) {
  if (json.length() < 16) {
    return false;
  }

  ConfigJsonDocument doc;
  const DeserializationError err = deserializeJson(doc, json);
  if (err) {
    HcSerial.print("[config] JSON parse failed: ");
    HcSerial.println(err.c_str());
    return false;
  }

  ctx.applyUserConfigDefaults();

  JsonObject owner = doc["owner"];
  if (!owner.isNull()) {
    strlcpy(ctx.config.ownerName, owner["name"] | ctx.config.ownerName, sizeof(ctx.config.ownerName));
    strlcpy(ctx.config.title, owner["title"] | ctx.config.title, sizeof(ctx.config.title));
    strlcpy(ctx.config.company, owner["company"] | ctx.config.company, sizeof(ctx.config.company));
    strlcpy(ctx.config.email, owner["email"] | ctx.config.email, sizeof(ctx.config.email));
    strlcpy(ctx.config.phone, owner["phone"] | ctx.config.phone, sizeof(ctx.config.phone));
    strlcpy(ctx.config.website, owner["website"] | ctx.config.website, sizeof(ctx.config.website));
    strlcpy(ctx.config.linkedin, owner["linkedin"] | ctx.config.linkedin, sizeof(ctx.config.linkedin));
    strlcpy(ctx.config.github, owner["github"] | ctx.config.github, sizeof(ctx.config.github));
    strlcpy(ctx.config.bio, owner["bio"] | ctx.config.bio, sizeof(ctx.config.bio));
  }

  JsonObject ap = doc["ap"];
  if (!ap.isNull()) {
    strlcpy(ctx.config.apSsid, ap["ssid"] | ctx.config.apSsid, sizeof(ctx.config.apSsid));
    strlcpy(ctx.config.apPassword, ap["password"] | ctx.config.apPassword, sizeof(ctx.config.apPassword));
    ctx.config.apChannel = ap["channel"] | ctx.config.apChannel;
  }

  JsonObject sta = doc["sta"];
  if (!sta.isNull()) {
    strlcpy(ctx.config.staSsid, sta["ssid"] | ctx.config.staSsid, sizeof(ctx.config.staSsid));
    strlcpy(ctx.config.staPassword, sta["password"] | ctx.config.staPassword, sizeof(ctx.config.staPassword));
    ctx.config.staCredentialsSaved = sta["saved"] | ctx.config.staCredentialsSaved;
  }

  JsonObject defaults = doc["defaults"];
  if (!defaults.isNull()) {
    ctx.config.ringBrightness = defaults["ring_brightness"] | ctx.config.ringBrightness;
    ctx.config.buzzerVolume = defaults["buzzer_volume"] | ctx.config.buzzerVolume;
    ctx.config.tuneSuccess = defaults["tune_success"] | ctx.config.tuneSuccess;
    ctx.config.tuneError = defaults["tune_error"] | ctx.config.tuneError;
    ctx.config.tuneBoot = defaults["tune_boot"] | ctx.config.tuneBoot;
    ctx.config.tuneNfc = defaults["tune_nfc"] | ctx.config.tuneNfc;
    ctx.config.bootRingPattern = defaults["boot_ring_pattern"] | ctx.config.bootRingPattern;
    ctx.config.bootRingR = defaults["boot_ring_r"] | ctx.config.bootRingR;
    ctx.config.bootRingG = defaults["boot_ring_g"] | ctx.config.bootRingG;
    ctx.config.bootRingB = defaults["boot_ring_b"] | ctx.config.bootRingB;
    ctx.config.wifiLedPattern = defaults["wifi_led_pattern"] | ctx.config.wifiLedPattern;
    ctx.config.wifiLedR = defaults["wifi_led_r"] | ctx.config.wifiLedR;
    ctx.config.wifiLedG = defaults["wifi_led_g"] | ctx.config.wifiLedG;
    ctx.config.wifiLedB = defaults["wifi_led_b"] | ctx.config.wifiLedB;
    ctx.config.wifiRingPattern = defaults["wifi_ring_pattern"] | ctx.config.wifiRingPattern;
    ctx.config.wifiRingR = defaults["wifi_ring_r"] | ctx.config.wifiRingR;
    ctx.config.wifiRingG = defaults["wifi_ring_g"] | ctx.config.wifiRingG;
    ctx.config.wifiRingB = defaults["wifi_ring_b"] | ctx.config.wifiRingB;
    ctx.config.bootAnimation = defaults["boot_animation"] | ctx.config.bootAnimation;
    ctx.config.buzzerEnabled = defaults["buzzer_enabled"] | ctx.config.buzzerEnabled;
    ctx.config.activityLogging = defaults["activity_logging"] | ctx.config.activityLogging;
    ctx.config.firstBootComplete = defaults["first_boot_complete"] | ctx.config.firstBootComplete;
    ctx.config.captivePortalEnabled = defaults["captive_portal_enabled"] | ctx.config.captivePortalEnabled;
    const char* modeName = defaults["mode"] | DeviceModeManager::toString(DeviceMode::Lab);
    ctx.config.deviceMode = static_cast<uint8_t>(DeviceModeManager::fromString(modeName));
    ctx.config.hidTargetOs = defaults["hid_target_os"] | ctx.config.hidTargetOs;
    strlcpy(ctx.config.labModePin, defaults["lab_mode_pin"] | ctx.config.labModePin, sizeof(ctx.config.labModePin));
  }

  JsonObject security = doc["security"];
  if (!security.isNull()) {
    strlcpy(ctx.config.labModePin, security["lab_mode_pin"] | ctx.config.labModePin, sizeof(ctx.config.labModePin));
  }

  return true;
}

String ConfigStore::serialize(const AppContext& ctx) const {
  ConfigJsonDocument doc;

  JsonObject owner = doc["owner"].to<JsonObject>();
  owner["name"] = ctx.config.ownerName;
  owner["title"] = ctx.config.title;
  owner["company"] = ctx.config.company;
  owner["email"] = ctx.config.email;
  owner["phone"] = ctx.config.phone;
  owner["website"] = ctx.config.website;
  owner["linkedin"] = ctx.config.linkedin;
  owner["github"] = ctx.config.github;
  owner["bio"] = ctx.config.bio;

  JsonObject ap = doc["ap"].to<JsonObject>();
  ap["ssid"] = ctx.config.apSsid;
  ap["password"] = ctx.config.apPassword;
  ap["channel"] = ctx.config.apChannel;

  JsonObject sta = doc["sta"].to<JsonObject>();
  sta["ssid"] = ctx.config.staSsid;
  sta["password"] = ctx.config.staPassword;
  sta["saved"] = ctx.config.staCredentialsSaved;

  JsonObject defaults = doc["defaults"].to<JsonObject>();
  defaults["ring_brightness"] = ctx.config.ringBrightness;
  defaults["buzzer_volume"] = ctx.config.buzzerVolume;
  defaults["tune_success"] = ctx.config.tuneSuccess;
  defaults["tune_error"] = ctx.config.tuneError;
  defaults["tune_boot"] = ctx.config.tuneBoot;
  defaults["tune_nfc"] = ctx.config.tuneNfc;
  defaults["boot_ring_pattern"] = ctx.config.bootRingPattern;
  defaults["boot_ring_r"] = ctx.config.bootRingR;
  defaults["boot_ring_g"] = ctx.config.bootRingG;
  defaults["boot_ring_b"] = ctx.config.bootRingB;
  defaults["wifi_led_pattern"] = ctx.config.wifiLedPattern;
  defaults["wifi_led_r"] = ctx.config.wifiLedR;
  defaults["wifi_led_g"] = ctx.config.wifiLedG;
  defaults["wifi_led_b"] = ctx.config.wifiLedB;
  defaults["wifi_ring_pattern"] = ctx.config.wifiRingPattern;
  defaults["wifi_ring_r"] = ctx.config.wifiRingR;
  defaults["wifi_ring_g"] = ctx.config.wifiRingG;
  defaults["wifi_ring_b"] = ctx.config.wifiRingB;
  defaults["boot_animation"] = ctx.config.bootAnimation;
  defaults["buzzer_enabled"] = ctx.config.buzzerEnabled;
  defaults["activity_logging"] = ctx.config.activityLogging;
  defaults["first_boot_complete"] = ctx.config.firstBootComplete;
  defaults["captive_portal_enabled"] = ctx.config.captivePortalEnabled;
  defaults["mode"] = DeviceModeManager::toString(DeviceModeManager::fromStored(ctx.config.deviceMode));
  defaults["hid_target_os"] = ctx.config.hidTargetOs;
  defaults["lab_mode_pin"] = ctx.config.labModePin;

  JsonObject security = doc["security"].to<JsonObject>();
  security["lab_mode_pin"] = ctx.config.labModePin;

  String output;
  if (serializeJsonPretty(doc, output) == 0 || output.length() < 16) {
    HcSerial.println("[config] JSON serialize failed");
    return String();
  }
  return output;
}

bool ConfigStore::loadJsonFromNvs(String& out) const {
  out = "";
  Preferences prefs;
  if (!prefs.begin(kNvsNamespace, true)) {
    return false;
  }
  out = prefs.getString(kNvsConfigKey, "");
  prefs.end();
  return out.length() >= 16;
}

bool ConfigStore::saveJsonToNvs(const String& json) const {
  if (json.length() < 16 || json.length() >= 3900) {
    return false;
  }
  Preferences prefs;
  if (!prefs.begin(kNvsNamespace, false)) {
    return false;
  }
  const size_t written = prefs.putString(kNvsConfigKey, json);
  prefs.end();
  return written > 0;
}

bool ConfigStore::save(const AppContext& ctx) {
  const String json = serialize(ctx);
  if (json.length() < 16) {
    return false;
  }

  const bool flashOk = Storage.writeTextFile(kConfigPath, json);
  const bool nvsOk = saveJsonToNvs(json);

  if (!flashOk && !nvsOk) {
    HcSerial.println("[config] Save failed (flash and NVS)");
    return false;
  }
  if (!flashOk) {
    HcSerial.println("[config] Flash save failed — config kept in NVS");
  }
  return true;
}

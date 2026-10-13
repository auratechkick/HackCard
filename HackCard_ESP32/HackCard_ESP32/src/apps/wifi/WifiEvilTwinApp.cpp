#include "WifiEvilTwinApp.h"

#include "config/app_registry.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/telemetry/StatusLed.h"
#if APP_WIFI_BEACON
#include "WifiBeaconApp.h"
#endif
#if APP_WIFI_PORTAL
#include "../../platform/hal/WifiCaptivePortal.h"
#endif

WifiEvilTwinApp WifiEvilTwin;

namespace {

constexpr uint8_t kMinDurationMin = 1;
constexpr uint8_t kMaxDurationMin = 15;
constexpr uint8_t kDefaultDurationMin = 5;

}  // namespace

bool WifiEvilTwinApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool WifiEvilTwinApp::validateTarget(const char* targetSsid, uint8_t channel, uint8_t durationMin, char* error,
                                     size_t errorLen) const {
  if (!targetSsid || targetSsid[0] == '\0') {
    strncpy(error, "Target SSID required", errorLen - 1);
    return false;
  }
  if (strlen(targetSsid) > 31) {
    strncpy(error, "SSID too long (max 31 chars)", errorLen - 1);
    return false;
  }
  if (channel != 0 && (channel < 1 || channel > 13)) {
    strncpy(error, "Channel must be 0 (auto) or 1-13", errorLen - 1);
    return false;
  }
  if (durationMin < kMinDurationMin || durationMin > kMaxDurationMin) {
    strncpy(error, "Duration must be 1-15 minutes", errorLen - 1);
    return false;
  }
  return true;
}

uint32_t WifiEvilTwinApp::getRemainingSec() const {
  if (!_running) {
    return 0;
  }
  const uint32_t now = millis();
  if (now >= _endsAtMs) {
    return 0;
  }
  return (_endsAtMs - now + 999) / 1000;
}

void WifiEvilTwinApp::restoreAp(AppContext& ctx) {
  if (!WifiAp.begin(_restoreSsid, _restorePassword, _restoreChannel)) {
    Log.error("wifi", "twin_restore failed");
    WifiAp.begin(ctx.config.apSsid, ctx.config.apPassword, ctx.config.apChannel);
  }

#if APP_WIFI_PORTAL
  WifiCaptive.setEnabled(_restorePortalEnabled);
#endif

  Status.setApActive(true);
  Status.setIdle();
}

WifiEvilTwinResult WifiEvilTwinApp::start(AppContext& ctx, const char* labPin, const char* targetSsid, uint8_t channel,
                                          uint8_t durationMin) {
  WifiEvilTwinResult result;

  if (_running) {
    strncpy(result.error, "Simulation already running", sizeof(result.error) - 1);
    return result;
  }

#if APP_WIFI_BEACON
  if (WifiBeacon.isRunning()) {
    strncpy(result.error, "Stop beacon lab first", sizeof(result.error) - 1);
    return result;
  }
#endif

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }

  if (durationMin == 0) {
    durationMin = kDefaultDurationMin;
  }

  if (!validateTarget(targetSsid, channel, durationMin, result.error, sizeof(result.error))) {
    return result;
  }

  strlcpy(_restoreSsid, ctx.config.apSsid, sizeof(_restoreSsid));
  strlcpy(_restorePassword, ctx.config.apPassword, sizeof(_restorePassword));
  _restoreChannel = ctx.config.apChannel;
  _restorePortalEnabled = ctx.config.captivePortalEnabled;

  strlcpy(_targetSsid, targetSsid, sizeof(_targetSsid));
  _channel = channel;

  if (!WifiAp.begin(_targetSsid, nullptr, _channel > 0 ? _channel : 6)) {
    strncpy(result.error, "Could not start simulation AP", sizeof(result.error) - 1);
    strncat(result.error, ": ", sizeof(result.error) - strlen(result.error) - 1);
    strncat(result.error, WifiAp.getLastError().c_str(), sizeof(result.error) - strlen(result.error) - 1);
    return result;
  }

#if APP_WIFI_PORTAL
  WifiCaptive.setEnabled(true);
#endif

  _durationSec = static_cast<uint32_t>(durationMin) * 60;
  _endsAtMs = millis() + (_durationSec * 1000);
  _running = true;

  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }

  Log.activity("wifi", "twin_start", _targetSsid);
  result.success = true;
  result.running = true;
  result.remainingSec = getRemainingSec();
  return result;
}

WifiEvilTwinResult WifiEvilTwinApp::stop(AppContext& ctx) {
  WifiEvilTwinResult result;

  if (!_running) {
    result.success = true;
    result.running = false;
    return result;
  }

  _running = false;
  restoreAp(ctx);

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }

  Log.activity("wifi", "twin_stop", _targetSsid);
  result.success = true;
  result.running = false;
  result.remainingSec = 0;
  return result;
}

void WifiEvilTwinApp::loop(AppContext& ctx) {
  if (!_running) {
    return;
  }

  if (millis() >= _endsAtMs) {
    Log.activity("wifi", "twin_expire", _targetSsid);
    stop(ctx);
  }
}

String WifiEvilTwinApp::statusToJson() const {
  String json = "{\"running\":";
  json += _running ? "true" : "false";
  json += ",\"target_ssid\":\"";
  json += _targetSsid;
  json += "\",\"restore_ap_ssid\":\"";
  json += _restoreSsid;
  json += "\",\"lab_ip\":\"192.168.4.1";
  json += "\",\"channel\":";
  json += String(_channel);
  json += ",\"remaining_sec\":";
  json += String(getRemainingSec());
  json += ",\"duration_sec\":";
  json += String(_durationSec);
  json += ",\"simulation\":true}";
  return json;
}

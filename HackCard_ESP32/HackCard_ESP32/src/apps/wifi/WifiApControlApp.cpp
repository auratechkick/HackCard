#include "WifiApControlApp.h"

#include "config/app_registry.h"
#include "../../core/ConfigStore.h"
#include "../../core/Logger.h"
#include "../../platform/hal/HalWifiAp.h"
#if APP_WIFI_PORTAL
#include "../../platform/hal/WifiCaptivePortal.h"
#endif

WifiApControlApp WifiApControl;

bool WifiApControlApp::validate(const char* ssid, const char* password, uint8_t channel, char* error,
                                size_t errorLen) const {
  if (!ssid || ssid[0] == '\0') {
    strncpy(error, "SSID cannot be empty", errorLen - 1);
    return false;
  }
  if (strlen(ssid) > 31) {
    strncpy(error, "SSID too long (max 31 chars)", errorLen - 1);
    return false;
  }
  if (password && password[0] != '\0' && strlen(password) < 8) {
    strncpy(error, "Password must be 8+ chars (or blank for open AP)", errorLen - 1);
    return false;
  }
  if (channel != 0 && (channel < 1 || channel > 13)) {
    strncpy(error, "Channel must be 0 (auto) or 1-13", errorLen - 1);
    return false;
  }
  return true;
}

WifiApStatus WifiApControlApp::getStatus(const HackCardConfig& config) const {
  WifiApStatus status;
  status.apRunning = WifiAp.verifyRunning();
  strlcpy(status.configSsid, config.apSsid, sizeof(status.configSsid));
  strlcpy(status.runningSsid, WifiAp.getSsid().c_str(), sizeof(status.runningSsid));
  strlcpy(status.ip, WifiAp.getIp().c_str(), sizeof(status.ip));
  status.configChannel = config.apChannel;
  status.runningChannel = static_cast<uint8_t>(WifiAp.getChannel());
  status.captivePortalEnabled = config.captivePortalEnabled;
  return status;
}

WifiApApplyResult WifiApControlApp::apply(AppContext& ctx, const char* ssid, const char* password, uint8_t channel) {
  WifiApApplyResult result;

  if (!validate(ssid, password, channel, result.error, sizeof(result.error))) {
    return result;
  }

  strlcpy(ctx.config.apSsid, ssid, sizeof(ctx.config.apSsid));
  if (password) {
    strlcpy(ctx.config.apPassword, password, sizeof(ctx.config.apPassword));
  } else {
    ctx.config.apPassword[0] = '\0';
  }
  ctx.config.apChannel = channel;

  if (!Config.save(ctx)) {
    strncpy(result.error, "Could not save config", sizeof(result.error) - 1);
    return result;
  }
  result.configSaved = true;

  if (!WifiAp.begin(ctx.config.apSsid, ctx.config.apPassword, ctx.config.apChannel)) {
    strncpy(result.error, "AP restart failed", sizeof(result.error) - 1);
    strncat(result.error, ": ", sizeof(result.error) - strlen(result.error) - 1);
    strncat(result.error, WifiAp.getLastError().c_str(), sizeof(result.error) - strlen(result.error) - 1);
    return result;
  }

  result.apRestarted = true;

#if APP_WIFI_PORTAL
  WifiCaptive.setEnabled(ctx.config.captivePortalEnabled);
#endif

  Log.activity("wifi", "ap_set", ctx.config.apSsid);
  result.success = true;
  return result;
}

String WifiApControlApp::statusToJson(const WifiApStatus& status) const {
  String json = "{\"ap_running\":";
  json += status.apRunning ? "true" : "false";
  json += ",\"config_ssid\":\"";
  json += status.configSsid;
  json += "\",\"running_ssid\":\"";
  json += status.runningSsid;
  json += "\",\"ip\":\"";
  json += status.ip;
  json += "\",\"config_channel\":";
  json += String(status.configChannel);
  json += ",\"running_channel\":";
  json += String(status.runningChannel);
  json += ",\"captive_portal\":";
  json += status.captivePortalEnabled ? "true" : "false";
  json += "}";
  return json;
}

String WifiApControlApp::applyToJson(const WifiApApplyResult& result, const WifiApStatus& status) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"config_saved\":";
  json += result.configSaved ? "true" : "false";
  json += ",\"ap_restarted\":";
  json += result.apRestarted ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += statusToJson(status);
  json += "}";
  return json;
}

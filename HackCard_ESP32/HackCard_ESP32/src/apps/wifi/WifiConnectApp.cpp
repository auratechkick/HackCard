#include "WifiConnectApp.h"

#include "../../core/AppContext.h"
#include "../../core/Logger.h"
#include "WifiStaStore.h"

extern AppContext gApp;

#include "../../core/StorageManager.h"

#include "../../platform/hal/HalBuzzer.h"

#include "../../platform/hal/HalWifiAp.h"

#include "../../platform/hal/WiFiHardwareTest.h"

#include "../../platform/telemetry/StatusLed.h"

#include "config/user_config.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

WifiConnectApp WifiConnect;



namespace {



void connectFeedbackStart() {

  Status.setScanning();

  if (Buzzer.isEnabled()) {

    Buzzer.playNfcReady();

  }

}



void connectFeedbackSuccess() {

  Status.setApActive(true);

  Status.setIdle();

  if (Buzzer.isEnabled()) {

    Buzzer.playSuccess();

  }

}



void connectFeedbackError() {

  Status.setError();

  if (Buzzer.isEnabled()) {

    Buzzer.playError();

  }

}



}  // namespace



bool WifiConnectApp::restoreAp() {

  WifiHwTest.disconnectSta();

  delay(100);

  if (!WifiAp.restart()) {

    return false;

  }

  Status.setApActive(true);

  return true;

}



bool WifiConnectApp::saveConnect(const WifiConnectResult& result) {

  Storage.ensureDirectory("/wifi");

  const String json = toJson(result);
  if (json.length() >= 16) {
    _memoryCacheJson = json;
  }
  return Storage.writeTextFile("/wifi/last_connect.json", json);

}



WifiConnectResult WifiConnectApp::connectOnce(const char* ssid, const char* password, uint8_t channel,

                                              const char* authLabel, const char* bssid) {

  WifiConnectResult result;

  if (!ssid || ssid[0] == '\0') {

    strncpy(result.error, "SSID required", sizeof(result.error) - 1);

    return result;

  }



  strlcpy(result.ssid, ssid, sizeof(result.ssid));

  result.channel = channel;

  connectFeedbackStart();



  const bool scanFirst = (channel == 0);

  const WiFiTestResult wifiResult =

      WifiHwTest.connectToNetwork(ssid, password, WIFI_TEST_CONNECT_TIMEOUT, channel, authLabel, bssid, scanFirst);



  if (!wifiResult.connectOk) {

    strncpy(result.error, wifiResult.failureReason.c_str(), sizeof(result.error) - 1);

    result.apRestored = restoreAp();

    if (result.apRestored) {

      Status.setApActive(true);

      Status.setIdle();

    }

    connectFeedbackError();

    saveConnect(result);

    return result;

  }



  result.success = true;

  strlcpy(result.ip, wifiResult.ip.c_str(), sizeof(result.ip));

  strlcpy(result.gateway, wifiResult.gateway.c_str(), sizeof(result.gateway));

  result.rssi = static_cast<int8_t>(WiFi.RSSI());



  delay(1500);

  saveConnect(result);
  WifiStaStoreSave(gApp, ssid, password);

  result.apRestored = restoreAp();

  if (!result.apRestored) {

    strncpy(result.error, "Connected but AP restart failed — run wifi restart", sizeof(result.error) - 1);

    connectFeedbackError();

    return result;

  }



  char summary[48];

  snprintf(summary, sizeof(summary), "%s @ %s", result.ssid, result.ip);

  Log.activity("wifi", "connect", summary);



  connectFeedbackSuccess();

  return result;

}



String WifiConnectApp::toJson(const WifiConnectResult& result) const {

  String json = "{\"ready\":true,\"success\":";

  json += result.success ? "true" : "false";

  json += ",\"ssid\":\"";

  json += result.ssid;

  json += "\",\"ip\":\"";

  json += result.ip;

  json += "\",\"gateway\":\"";

  json += result.gateway;

  json += "\",\"rssi\":";

  json += String(result.rssi);

  json += ",\"channel\":";

  json += String(result.channel);

  json += ",\"ap_restored\":";

  json += result.apRestored ? "true" : "false";

  json += ",\"error\":\"";

  json += result.error;

  json += "\"}";

  return json;

}



String WifiConnectApp::toText(const WifiConnectResult& result) const {

  if (!result.success) {

    String text = "── Wi-Fi Connect FAILED ──\n";

    text += "SSID: ";

    text += result.ssid;

    text += "\n";

    text += result.error;

    text += "\n";

    return text;

  }



  String text = "── Wi-Fi Connect OK ──\n";

  text += "SSID:    ";

  text += result.ssid;

  text += "\nIP:      ";

  text += result.ip;

  text += "\nGateway: ";

  text += result.gateway;

  text += "\nRSSI:    ";

  text += String(result.rssi);

  text += " dBm\n";

  return text;

}



String WifiConnectApp::readLastConnectJson() const {

  String raw;

  if (!Storage.readTextFile("/wifi/last_connect.json", raw)) {

    return "{\"ready\":false,\"success\":false,\"error\":\"No saved connect\"}";

  }

  return raw;

}



bool WifiConnectApp::hasLastConnect() const {

  return Storage.exists("/wifi/last_connect.json");

}

namespace {

TaskHandle_t gConnectWorkerTask = nullptr;

}  // namespace

bool WifiConnectApp::requestAsyncConnect(const char* ssid, const char* password, uint8_t channel,
                                         const char* authLabel, const char* bssid) {
  if (_connectActive || gConnectWorkerTask != nullptr) {
    return false;
  }
  if (!ssid || ssid[0] == '\0') {
    return false;
  }

  strlcpy(_pendingSsid, ssid, sizeof(_pendingSsid));
  strlcpy(_pendingPassword, password ? password : "", sizeof(_pendingPassword));
  _pendingChannel = channel;
  strlcpy(_pendingAuth, authLabel ? authLabel : "", sizeof(_pendingAuth));
  strlcpy(_pendingBssid, bssid ? bssid : "", sizeof(_pendingBssid));
  _connectActive = true;

  if (xTaskCreatePinnedToCore(connectWorkerEntry, "hcWifiConn", 12288, this, 1, &gConnectWorkerTask, 0) != pdPASS) {
    _connectActive = false;
    return false;
  }
  return true;
}

void WifiConnectApp::connectWorkerEntry(void* param) {
  WifiConnectApp* self = static_cast<WifiConnectApp*>(param);
  if (self) {
    self->runConnectWorker();
  }
  gConnectWorkerTask = nullptr;
  vTaskDelete(nullptr);
}

void WifiConnectApp::runConnectWorker() {
  const WifiConnectResult result =
      connectOnce(_pendingSsid, _pendingPassword, _pendingChannel,
                  _pendingAuth[0] ? _pendingAuth : nullptr, _pendingBssid[0] ? _pendingBssid : nullptr);
  _memoryCacheJson = toJson(result);
  _connectActive = false;
}

String WifiConnectApp::lastConnectApiJson() const {
  if (_connectActive) {
    return "{\"ready\":false,\"connecting\":true,\"success\":false,\"error\":\"Connect in progress\"}";
  }
  if (_memoryCacheJson.length() >= 16) {
    String json = _memoryCacheJson;
    if (json.indexOf("\"connecting\"") < 0) {
      json.remove(json.length() - 1);
      json += ",\"connecting\":false}";
    }
    return json;
  }
  return readLastConnectJson();
}


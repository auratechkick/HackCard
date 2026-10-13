#include "WebStatusServer.h"

#include "../../core/HcSerial.h"
#include "../../core/LabPin.h"
#include "../../core/SerialLogBuffer.h"
#include "config/app_registry.h"
#include "config/board_config.h"
#include "DiagnosticsApp.h"
#include "ActivityLog.h"
#include "../demo/LedBuzzerApp.h"
#if APP_USB_HID
#include "../usb/HidLabApp.h"
#include "../usb/HidPayloadStore.h"
#endif
#include "../nfc/NfcReadApp.h"
#include "../nfc/NfcInfoApp.h"
#include "../nfc/NfcDumpApp.h"
#include "../nfc/NfcWriteApp.h"
#include "../nfc/NfcHistory.h"
#include "../nfc/NfcLabApp.h"
#include "../nfc/NfcClassicApp.h"
#if APP_WIFI_SCANNER
#include "../wifi/WifiScannerApp.h"
#endif
#if APP_WIFI_AP_CONTROL
#include "../wifi/WifiApControlApp.h"
#endif
#if APP_WIFI_BEACON
#include "../wifi/WifiBeaconApp.h"
#endif
#if APP_WIFI_EVIL_TWIN
#include "../wifi/WifiEvilTwinApp.h"
#endif
#if APP_WIFI_TRAINING
#include "../wifi/WifiTrainingPortalApp.h"
#endif
#if APP_WIFI_MONITOR
#include "../wifi/WifiMonitorApp.h"
#endif
#if APP_WIFI_RECON
#include "../wifi/WifiReconApp.h"
#endif
#if APP_WIFI_KARMA
#include "../wifi/WifiKarmaApp.h"
#endif
#if APP_WIFI_CAPTURE
#include "../wifi/WifiCaptureApp.h"
#endif
#if APP_BLE
#include "../ble/BleLabApp.h"
#include "../ble/BleContactCardApp.h"
#endif
#include "../../core/BootState.h"
#include "../../core/DeviceModeManager.h"
#include "../../platform/hal/HalWifiAp.h"

#include <WiFi.h>
#include "../../platform/hal/WifiCaptivePortal.h"
#include "../../core/CliEngine.h"
#include "../../core/ConfigStore.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalRgbRing.h"
#include "../../platform/hal/HalWifiLed.h"

#include <WebServer.h>
#include <WiFi.h>

#if APP_WEB_UI
#include "WebUiEmbedded.h"
#endif

WebStatusServer WebStatus;

namespace {

WebServer sServer(80);

#if APP_WEB_UI

const char* mimeTypeForPath(const String& path) {
  if (path.endsWith(".css")) {
    return "text/css";
  }
  if (path.endsWith(".js")) {
    return "application/javascript";
  }
  if (path.endsWith(".svg")) {
    return "image/svg+xml";
  }
  if (path.endsWith(".png")) {
    return "image/png";
  }
  if (path.endsWith(".ico")) {
    return "image/x-icon";
  }
  return "text/html";
}

String normalizeUiUri(const String& uri) {
  if (uri == "/admin" || uri == "/admin.html") {
    return "/";
  }
  return uri;
}

void sendProgmemBody(const char* mime, const uint8_t* data, size_t len) {
  sServer.setContentLength(len);
  sServer.send(200, mime, "");
  WiFiClient client = sServer.client();
  uint8_t chunk[512];
  for (size_t offset = 0; offset < len; offset += sizeof(chunk)) {
    const size_t n = min(sizeof(chunk), len - offset);
    memcpy_P(chunk, data + offset, n);
    client.write(chunk, n);
  }
}

bool tryServeEmbeddedStatic(const String& uri) {
  const String requestUri = normalizeUiUri(uri);
  for (size_t i = 0; i < kWebUiFileCount; i++) {
    if (requestUri != kWebUiFiles[i].uri) {
      continue;
    }
    sendProgmemBody(kWebUiFiles[i].mime, kWebUiFiles[i].data, kWebUiFiles[i].len);
    return true;
  }
  return false;
}

bool tryServeWwwStatic(const String& uri) {
  const String requestUri = normalizeUiUri(uri);
  String path;
  if (requestUri == "/") {
    path = "/www/index.html";
  } else if (requestUri == "/profile.html") {
    path = "/www/profile.html";
  } else if (requestUri.startsWith("/assets/")) {
    path = "/www" + requestUri;
  } else {
    return false;
  }

  String body;
  if (Storage.readTextFile(path.c_str(), body) && body.length() > 0) {
    sServer.send(200, mimeTypeForPath(path), body);
    return true;
  }
  if (requestUri == "/profile.html") {
    return tryServeEmbeddedStatic("/profile.html");
  }
  return tryServeEmbeddedStatic(requestUri);
}

void sendUiMissingHelp() {
  sServer.send(503, "text/html",
               "<html><body style='font-family:sans-serif;background:#111;color:#eee;padding:24px'>"
               "<h1>HackCard UI unavailable</h1>"
               "<p>Rebuild firmware after running <code>scripts/sync-web-ui.ps1</code>.</p>"
               "<p>API still works at <code>/api/status</code>.</p>"
               "</body></html>");
}

void handleDashboardUi() {
  if (!tryServeWwwStatic("/")) {
    sendUiMissingHelp();
  }
}
#endif

}  // namespace

bool WebStatusServer::begin(AppContext* ctx) {
#if !APP_SYSTEM_WEB
  (void)ctx;
  return false;
#else
  _ctx = ctx;
  if (!_ctx) {
    return false;
  }

  WiFi.persistent(false);

  if (!WifiAp.begin(_ctx->config.apSsid, _ctx->config.apPassword, _ctx->config.apChannel)) {
    HcSerial.println("[web] ERROR: WiFi AP failed to start");
    return false;
  }

#if APP_WEB_UI
  sServer.on("/", [this]() { handleRootRoute(); });
  sServer.on("/lab", [this]() { handleLabConsoleUi(); });
  sServer.on("/admin", []() {
    sServer.sendHeader("Location", "/");
    sServer.send(302, "text/plain", "Redirecting\n");
  });
  sServer.on("/admin.html", []() {
    sServer.sendHeader("Location", "/");
    sServer.send(302, "text/plain", "Redirecting\n");
  });
  sServer.on("/setup", [this]() { handleLabConsoleUi(); });
#if APP_PRO_PROFILE
  sServer.on("/profile", [this]() { handleLabConsoleUi(); });
  sServer.on("/profile/edit", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/settings", [this]() { handleLabConsoleUi(); });
#if APP_DEMO_LED_BUZZER
  sServer.on("/apps", [this]() { handleLabConsoleUi(); });
#endif
#if APP_SYSTEM_LOGS
  sServer.on("/logs", [this]() { handleLabConsoleUi(); });
#endif
#if APP_MODES
  sServer.on("/modes", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/diag", [this]() { handleLabConsoleUi(); });
  sServer.on("/terminal", [this]() { handleLabConsoleUi(); });
  sServer.on("/profile.html", [this]() { handleBusinessProfile(); });
#endif
#if APP_PRO_PROFILE
  sServer.on("/profile.vcf", [this]() {
    sServer.send(200, "text/vcard", buildVcard(_ctx->config));
  });
#endif
#if APP_DEMO_LED_BUZZER
  sServer.on("/api/apps", HTTP_POST, [this]() { handleAppsApi(); });
#endif
#if APP_SYSTEM_LOGS
  sServer.on("/api/logs", HTTP_GET, [this]() { handleLogsApi(); });
  sServer.on("/api/logs/clear", HTTP_POST, [this]() { handleLogsClearApi(); });
#endif
#if APP_MODES
  sServer.on("/api/mode", HTTP_POST, [this]() { handleModeApi(); });
  sServer.on("/api/modes", HTTP_GET, [this]() { handleModesGetApi(); });
#endif
  sServer.on("/api/lab/unlock", HTTP_POST, [this]() { handleLabUnlockApi(); });
#if APP_NFC_READ
#if APP_WEB_UI
  sServer.on("/nfc", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/nfc/status", HTTP_GET, [this]() { handleNfcStatusApi(); });
  sServer.on("/api/nfc/read", HTTP_POST, [this]() { handleNfcReadApi(); });
  sServer.on("/api/nfc/info", HTTP_POST, [this]() { handleNfcInfoApi(); });
#if APP_NFC_DUMP
  sServer.on("/api/nfc/dump", HTTP_POST, [this]() { handleNfcDumpApi(); });
#endif
#if APP_NFC_WRITE
  sServer.on("/api/nfc/write", HTTP_POST, [this]() { handleNfcWriteApi(); });
  sServer.on("/api/nfc/erase", HTTP_POST, [this]() { handleNfcEraseApi(); });
#endif
  sServer.on("/api/nfc/history", HTTP_GET, [this]() { handleNfcHistoryApi(); });
  sServer.on("/api/nfc/last-dump", HTTP_GET, [this]() { handleNfcLastDumpApi(); });
#if APP_NFC_CLONE
  sServer.on("/api/nfc/clone", HTTP_POST, [this]() { handleNfcCloneApi(); });
#endif
#if APP_NFC_KILL
  sServer.on("/api/nfc/kill", HTTP_POST, [this]() { handleNfcKillApi(); });
#endif
#if APP_NFC_CLASSIC
  sServer.on("/api/nfc/classic/dump", HTTP_POST, [this]() { handleNfcClassicDumpApi(); });
  sServer.on("/api/nfc/classic/clone", HTTP_POST, [this]() { handleNfcClassicCloneApi(); });
  sServer.on("/api/nfc/keys/scan", HTTP_POST, [this]() { handleNfcKeysScanApi(); });
  sServer.on("/api/nfc/storage/prefs", HTTP_POST, [this]() { handleNfcStoragePrefsApi(); });
  sServer.on("/api/nfc/history/export/sd", HTTP_POST, [this]() { handleNfcHistoryExportSdApi(); });
  sServer.on("/api/nfc/dump/export/sd", HTTP_POST, [this]() { handleNfcDumpExportSdApi(); });
  sServer.on("/api/nfc/classic/export/sd", HTTP_POST, [this]() { handleNfcClassicExportSdApi(); });
#endif
#endif
#if APP_WIFI_SCANNER
#if APP_WEB_UI
  sServer.on("/wifi", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/wifi/scan/clear", HTTP_POST, [this]() { handleWifiScanClearApi(); });
  sServer.on("/api/wifi/scan", HTTP_POST, [this]() { handleWifiScanApi(); });
  sServer.on("/api/wifi/last", HTTP_GET, [this]() { handleWifiLastApi(); });
#endif
#if APP_WIFI_AP_CONTROL
#if APP_WEB_UI
  sServer.on("/wifi/ap", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/wifi/ap", HTTP_GET, [this]() { handleWifiApGetApi(); });
  sServer.on("/api/wifi/ap", HTTP_POST, [this]() { handleWifiApPostApi(); });
#endif
#if APP_WIFI_BEACON
  sServer.on("/api/wifi/beacon", HTTP_GET, [this]() { handleWifiBeaconStatusApi(); });
  sServer.on("/api/wifi/beacon/start", HTTP_POST, [this]() { handleWifiBeaconStartApi(); });
  sServer.on("/api/wifi/beacon/stop", HTTP_POST, [this]() { handleWifiBeaconStopApi(); });
#endif
#if APP_WIFI_EVIL_TWIN
  sServer.on("/api/wifi/twin", HTTP_GET, [this]() { handleWifiTwinStatusApi(); });
  sServer.on("/api/wifi/twin/start", HTTP_POST, [this]() { handleWifiTwinStartApi(); });
  sServer.on("/api/wifi/twin/stop", HTTP_POST, [this]() { handleWifiTwinStopApi(); });
#endif
#if APP_WIFI_TRAINING
#if APP_WEB_UI
  sServer.on("/wifi/training", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/portal/reveal", [this]() { sServer.send(200, "text/html", buildTrainingRevealPage()); });
  sServer.on("/api/wifi/training", HTTP_GET, [this]() { handleWifiTrainingStatusApi(); });
  sServer.on("/api/wifi/training/config", HTTP_POST, [this]() { handleWifiTrainingConfigApi(); });
  sServer.on("/api/wifi/training/start", HTTP_POST, [this]() { handleWifiTrainingStartApi(); });
  sServer.on("/api/wifi/training/stop", HTTP_POST, [this]() { handleWifiTrainingStopApi(); });
  sServer.on("/api/wifi/training/submit", HTTP_POST, [this]() { handleWifiTrainingSubmitApi(); });
  sServer.on("/api/wifi/training/captures", HTTP_GET, [this]() { handleWifiTrainingCapturesApi(); });
  sServer.on("/api/wifi/training/captures/clear", HTTP_POST, [this]() { handleWifiTrainingCapturesClearApi(); });
  sServer.on("/api/wifi/training/export", HTTP_GET, [this]() { handleWifiTrainingExportApi(); });
  sServer.on("/api/wifi/training/export/sd", HTTP_POST, [this]() { handleWifiTrainingExportSdApi(); });
#endif
#if APP_WIFI_MONITOR
#if APP_WEB_UI
  sServer.on("/monitor", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/wifi/monitor", HTTP_GET, [this]() { handleWifiMonitorStatusApi(); });
  sServer.on("/api/wifi/monitor/probes", HTTP_GET, [this]() { handleWifiMonitorProbesApi(); });
  sServer.on("/api/wifi/monitor/live", HTTP_GET, [this]() { handleWifiMonitorLiveApi(); });
  sServer.on("/api/wifi/monitor/sniff/start", HTTP_POST, [this]() { handleWifiMonitorSniffStartApi(); });
  sServer.on("/api/wifi/monitor/sniff/stop", HTTP_POST, [this]() { handleWifiMonitorSniffStopApi(); });
  sServer.on("/api/wifi/monitor/deauth/start", HTTP_POST, [this]() { handleWifiMonitorDeauthStartApi(); });
  sServer.on("/api/wifi/monitor/deauth/stop", HTTP_POST, [this]() { handleWifiMonitorDeauthStopApi(); });
  sServer.on("/api/wifi/monitor/probes/clear", HTTP_POST, [this]() { handleWifiMonitorProbesClearApi(); });
  sServer.on("/api/wifi/monitor/probes/export/sd", HTTP_POST, [this]() { handleWifiMonitorProbesExportSdApi(); });
  sServer.on("/api/wifi/monitor/aps", HTTP_GET, [this]() { handleWifiMonitorApsApi(); });
#endif
#if APP_WIFI_SCANNER || APP_WIFI_RECON
  sServer.on("/api/wifi/sta", HTTP_POST, [this]() { handleWifiStaGetApi(); });
#endif
#if APP_WIFI_RECON
#if APP_WEB_UI
  sServer.on("/wifi/recon", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/wifi/recon", HTTP_GET, [this]() { handleWifiReconStatusApi(); });
  sServer.on("/api/wifi/recon/connect", HTTP_POST, [this]() { handleWifiReconConnectApi(); });
  sServer.on("/api/wifi/recon/release", HTTP_POST, [this]() { handleWifiReconReleaseApi(); });
  sServer.on("/api/wifi/recon/subnet", HTTP_POST, [this]() { handleWifiReconSubnetApi(); });
  sServer.on("/api/wifi/recon/hosts", HTTP_GET, [this]() { handleWifiReconHostsApi(); });
  sServer.on("/api/wifi/recon/portscan", HTTP_POST, [this]() { handleWifiReconPortScanApi(); });
  sServer.on("/api/wifi/recon/ports", HTTP_GET, [this]() { handleWifiReconPortsApi(); });
#endif
#if APP_WIFI_KARMA
  sServer.on("/api/wifi/karma", HTTP_GET, [this]() { handleWifiKarmaStatusApi(); });
  sServer.on("/api/wifi/karma/start", HTTP_POST, [this]() { handleWifiKarmaStartApi(); });
  sServer.on("/api/wifi/karma/stop", HTTP_POST, [this]() { handleWifiKarmaStopApi(); });
#endif
#if APP_WIFI_CAPTURE
  sServer.on("/api/wifi/capture", HTTP_GET, [this]() { handleWifiCaptureStatusApi(); });
  sServer.on("/api/wifi/capture/start", HTTP_POST, [this]() { handleWifiCaptureStartApi(); });
  sServer.on("/api/wifi/capture/stop", HTTP_POST, [this]() { handleWifiCaptureStopApi(); });
  sServer.on("/api/wifi/capture/download", HTTP_GET, [this]() { handleWifiCaptureDownloadApi(); });
  sServer.on("/api/wifi/capture/verify", HTTP_POST, [this]() { handleWifiCaptureVerifyApi(); });
  sServer.on("/api/wifi/capture/export/sd", HTTP_POST, [this]() { handleWifiCaptureExportSdApi(); });
#endif
#if APP_BLE
#if APP_WEB_UI
  sServer.on("/ble", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/ble/status", HTTP_GET, [this]() { handleBleStatusApi(); });
  sServer.on("/api/ble/scan", HTTP_POST, [this]() { handleBleScanApi(); });
  sServer.on("/api/ble/scan/last", HTTP_GET, [this]() { handleBleLastScanApi(); });
  sServer.on("/api/ble/advertise/start", HTTP_POST, [this]() { handleBleAdvertiseStartApi(); });
  sServer.on("/api/ble/advertise/stop", HTTP_POST, [this]() { handleBleAdvertiseStopApi(); });
  sServer.on("/api/ble/contact/start", HTTP_POST, [this]() { handleBleContactStartApi(); });
  sServer.on("/api/ble/contact/stop", HTTP_POST, [this]() { handleBleContactStopApi(); });
  sServer.on("/api/ble/contact/refresh", HTTP_POST, [this]() { handleBleContactRefreshApi(); });
#endif
#if APP_USB_HID
#if APP_WEB_UI
  sServer.on("/hid", [this]() { handleLabConsoleUi(); });
#endif
  sServer.on("/api/hid/status", HTTP_GET, [this]() { handleHidStatusApi(); });
  sServer.on("/api/hid/device", HTTP_POST, [this]() { handleHidDeviceApi(); });
  sServer.on("/api/hid/os", HTTP_POST, [this]() { handleHidOsApi(); });
  sServer.on("/api/hid/run", HTTP_POST, [this]() { handleHidRunApi(); });
  sServer.on("/api/hid/payloads", HTTP_GET, [this]() { handleHidPayloadsApi(); });
  sServer.on("/api/hid/payload", HTTP_GET, [this]() { handleHidPayloadReadApi(); });
  sServer.on("/api/hid/payload/save", HTTP_POST, [this]() { handleHidPayloadSaveApi(); });
  sServer.on("/api/hid/payload/save/sd", HTTP_POST, [this]() { handleHidPayloadSaveSdApi(); });
  sServer.on("/api/hid/payload/delete", HTTP_POST, [this]() { handleHidPayloadDeleteApi(); });
  sServer.on("/api/hid/run-script", HTTP_POST, [this]() { handleHidRunScriptApi(); });
  sServer.on("/api/hid/arm", HTTP_POST, [this]() { handleHidArmApi(); });
#endif
#if APP_WIFI_PORTAL
  sServer.on("/portal", [this]() { handleBusinessProfile(); });
  sServer.on("/generate_204", [this]() { handleCaptiveDetect(); });
  sServer.on("/hotspot-detect.html", [this]() { handleCaptiveDetect(); });
  sServer.on("/library/test/success.html", [this]() { handleCaptiveDetect(); });
  sServer.on("/connecttest.txt", [this]() { handleCaptiveDetect(); });
  sServer.on("/ncsi.txt", [this]() { handleCaptiveDetect(); });
  sServer.on("/redirect", [this]() { handleCaptiveRedirect(); });
#endif
  sServer.on("/api/status", [this]() { sServer.send(200, "application/json", buildStatusJson()); });
  sServer.on("/api/setup", HTTP_POST, [this]() { handleSetupApi(); });
  sServer.on("/api/profile/public", HTTP_GET, [this]() { handleProfilePublicApi(); });
  sServer.on("/api/profile", HTTP_POST, [this]() { handleProfileApi(); });
  sServer.on("/api/settings", HTTP_POST, [this]() { handleSettingsApi(); });
  sServer.on("/api/diag/run", HTTP_POST, [this]() { handleDiagApi(); });
  sServer.on("/api/cli/banner", HTTP_GET, [this]() { handleCliBanner(); });
  sServer.on("/api/cli", HTTP_POST, [this]() { handleCliApi(); });
  sServer.on("/api/log/serial", HTTP_GET, [this]() { handleSerialLogApi(); });
  sServer.onNotFound([this]() { handleNotFound(); });
  sServer.begin();

#if APP_WIFI_PORTAL
  WifiCaptive.begin(_ctx->config.captivePortalEnabled);
#endif
#if APP_WIFI_BEACON
  WifiBeacon.begin();
#endif
#if APP_WIFI_TRAINING
  WifiTraining.begin();
#endif
#if APP_WIFI_MONITOR
  WifiMonitor.begin();
#endif
#if APP_WIFI_KARMA
  WifiKarma.begin();
#endif
#if APP_WIFI_CAPTURE
  WifiCapture.begin();
#endif

  _running = true;
  return true;
#endif
}

void WebStatusServer::loop() {
#if APP_SYSTEM_WEB
  if (_running) {
    WifiAp.maintain();
#if APP_WIFI_PORTAL
    WifiCaptive.loop();
#endif
#if APP_WIFI_BEACON
    WifiBeacon.loop();
#endif
#if APP_WIFI_EVIL_TWIN
    if (_ctx) {
      WifiEvilTwin.loop(*_ctx);
    }
#endif
#if APP_WIFI_MONITOR
    WifiMonitor.loop();
#endif
#if APP_WIFI_KARMA
    WifiKarma.loop();
#endif
#if APP_WIFI_CAPTURE
    WifiCapture.loop();
#endif
#if APP_WIFI_SCANNER
    WifiScanner.loop();
#endif
    sServer.handleClient();
  }
#endif
}

String WebStatusServer::buildStatusJson() const {
  if (!_ctx) {
    return "{}";
  }

  auto esc = [](const char* text) -> String {
    String out;
    if (!text) {
      return out;
    }
    for (const char* p = text; *p; ++p) {
      const char c = *p;
      if (c == '\\' || c == '"') {
        out += '\\';
        out += c;
      } else if (c == '\n') {
        out += "\\n";
      } else if (c == '\r') {
        out += "\\r";
      } else {
        out += c;
      }
    }
    return out;
  };

  const uint32_t uptimeSec = millis() / 1000;
  String json = "{";
  json += "\"firmware\":\"" + esc(FIRMWARE_VERSION) + "\",";
  json += "\"owner\":\"" + esc(_ctx->config.ownerName) + "\",";
  json += "\"owner_name\":\"" + esc(_ctx->config.ownerName) + "\",";
  json += "\"title\":\"" + esc(_ctx->config.title) + "\",";
  json += "\"email\":\"" + esc(_ctx->config.email) + "\",";
  json += "\"storage\":\"" + esc(Storage.getTierLabel()) + "\",";
  json += "\"config_source\":\"" + esc(Config.getSourceLabel()) + "\",";
  json += "\"first_boot_complete\":" + String(_ctx->config.firstBootComplete ? "true" : "false") + ",";
  const DeviceMode mode = DeviceModeManager::fromStored(_ctx->config.deviceMode);
  const DeviceModeSpec modeSpec = DeviceModeManager::spec(mode);
  json += "\"mode\":\"" + esc(DeviceModeManager::toString(mode)) + "\",";
  json += "\"mode_title\":\"" + esc(modeSpec.title) + "\",";
  json += "\"mode_description\":\"" + esc(modeSpec.description) + "\",";
  json += "\"business_mode\":" + String(DeviceModeManager::isBusinessMode(mode) ? "true" : "false") + ",";
  json += "\"lab_access\":" + String(hasLabAccess() ? "true" : "false") + ",";
  json += "\"uptime_ms\":" + String(millis()) + ",";
  json += "\"uptime\":" + String(uptimeSec) + ",";
  json += "\"clients\":" + String(WiFi.softAPgetStationNum()) + ",";
  json += "\"ap_ssid\":\"" + esc(_ctx->config.apSsid) + "\",";
  json += "\"ap_ip\":\"" + esc(WifiAp.getIp().c_str()) + "\",";
  json += "\"brightness\":" + String(_ctx->config.ringBrightness) + ",";
  json += "\"buzzer_volume\":" + String(_ctx->config.buzzerVolume) + ",";
#if APP_DEMO_LED_BUZZER
  json += "\"melody_playing\":" + String(LedBuzzer.isBuzzerMelodyPlaying() ? "true" : "false") + ",";
#else
  json += "\"melody_playing\":false,";
#endif
  json += "\"buzzer_enabled\":" + String(_ctx->config.buzzerEnabled ? "true" : "false") + ",";
  json += "\"boot_animation\":" + String(_ctx->config.bootAnimation ? "true" : "false") + ",";
  json += "\"activity_logging\":" + String(_ctx->config.activityLogging ? "true" : "false") + ",";
  json += "\"tune_success\":" + String(_ctx->config.tuneSuccess) + ",";
  json += "\"tune_error\":" + String(_ctx->config.tuneError) + ",";
  json += "\"tune_boot\":" + String(_ctx->config.tuneBoot) + ",";
  json += "\"tune_nfc\":" + String(_ctx->config.tuneNfc) + ",";
  json += "\"boot_ring_pattern\":" + String(_ctx->config.bootRingPattern) + ",";
  json += "\"boot_ring_r\":" + String(_ctx->config.bootRingR) + ",";
  json += "\"boot_ring_g\":" + String(_ctx->config.bootRingG) + ",";
  json += "\"boot_ring_b\":" + String(_ctx->config.bootRingB);
  json += ",\"wifi_led_pattern\":" + String(_ctx->config.wifiLedPattern);
  json += ",\"wifi_led_r\":" + String(_ctx->config.wifiLedR);
  json += ",\"wifi_led_g\":" + String(_ctx->config.wifiLedG);
  json += ",\"wifi_led_b\":" + String(_ctx->config.wifiLedB);
  json += ",\"wifi_ring_pattern\":" + String(_ctx->config.wifiRingPattern);
  json += ",\"wifi_ring_r\":" + String(_ctx->config.wifiRingR);
  json += ",\"wifi_ring_g\":" + String(_ctx->config.wifiRingG);
  json += ",\"wifi_ring_b\":" + String(_ctx->config.wifiRingB);
  json += ",\"sta_saved\":" + String(_ctx->config.staCredentialsSaved ? "true" : "false");
  json += ",\"sta_ssid\":\"" + esc(_ctx->config.staSsid) + "\"";
  json += ",\"sta_connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
  if (WiFi.status() == WL_CONNECTED) {
    json += ",\"sta_ip\":\"" + esc(WiFi.localIP().toString().c_str()) + "\"";
    json += ",\"sta_rssi\":" + String(WiFi.RSSI());
  }
  json += ",\"dashboard_url\":\"http://" + esc(WifiAp.getIp().c_str()) + "/lab\"";
#if APP_WIFI_PORTAL
  json += ",";
  json += "\"captive_portal\":" + String(_ctx->config.captivePortalEnabled ? "true" : "false");
#endif
  if (hasLabAccess()) {
    json += ",\"lab_pin\":\"" + esc(_ctx->config.labModePin) + "\"";
  }
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"flash_free_bytes\":";
  json += String(static_cast<unsigned>(Storage.freeFlashBytes()));
  json += ",\"storage_status\":\"" + esc(Storage.getStatusLine()) + "\"";
  json += "}";
  return json;
}

bool WebStatusServer::respondSdExportUnavailable() {
  if (Storage.isSdReady()) {
    return false;
  }
  char error[128] = {0};
  Storage.canExportToSd(error, sizeof(error));
  String json = "{\"success\":false,\"sd_ready\":false,\"error\":\"";
  json += error;
  json += "\"}";
  sServer.send(200, "application/json", json);
  return true;
}


String WebStatusServer::buildPortalPage() const {
  if (!_ctx) {
    return "<html><body>Not ready</body></html>";
  }

#if APP_WIFI_TRAINING
  if (WifiTraining.isActive()) {
    return WifiTraining.buildPortalHtml(WifiAp.getIp());
  }
#endif

  const HackCardConfig& c = _ctx->config;
  const String ip = WifiAp.getIp();

  String html;
  html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>";
  html += c.ownerName;
  html += "</title><style>";
  html += "body{margin:0;font-family:Arial,sans-serif;background:#0a0a0a;color:#fff;min-height:100vh;}";
  html += ".wrap{max-width:420px;margin:0 auto;padding:28px 20px 40px;text-align:center;}";
  html += ".logo{font-size:13px;letter-spacing:.08em;margin-bottom:18px;font-weight:800;}";
  html += ".logo .w{color:#fff;}.logo .r{color:#e02020;}";
  html += ".avatar{width:88px;height:88px;border-radius:18px;background:linear-gradient(145deg,#b81818,#e02020);margin:0 auto 16px;";
  html += "display:flex;align-items:center;justify-content:center;font-size:36px;font-weight:700;color:#fff;}";
  html += "h1{margin:0 0 6px;font-size:28px;color:#fff;}";
  html += ".title{color:#a3a3a3;font-size:16px;margin-bottom:18px;}";
  html += ".bio{color:#d4d4d4;font-size:14px;line-height:1.5;margin-bottom:22px;}";
  html += ".btn{display:block;background:#e02020;color:#fff;text-decoration:none;border-radius:12px;padding:14px 16px;";
  html += "font-weight:700;margin:10px 0;}";
  html += ".btn.secondary{background:#1a1a1a;border:1px solid #2a2a2a;color:#fff;}";
  html += ".links{display:flex;flex-wrap:wrap;gap:10px;justify-content:center;margin-top:16px;}";
  html += ".links a{color:#e02020;text-decoration:none;font-size:14px;padding:8px 12px;background:#111;border-radius:8px;border:1px solid #2a2a2a;}";
  html += ".foot{margin-top:28px;font-size:12px;color:#6e7681;}";
  html += ".sim-banner{background:#5a1a1a;border:2px solid #f85149;color:#ffd7d5;border-radius:12px;padding:14px 16px;";
  html += "margin-bottom:20px;font-size:14px;font-weight:700;line-height:1.4;}";
  html += "</style></head><body><div class='wrap'>";
#if APP_WIFI_EVIL_TWIN
  if (WifiEvilTwin.isRunning()) {
    html += "<div class='sim-banner'>SIMULATION ONLY — Authorized lab / training.<br>";
    html += "Fake AP: <strong>";
    html += WifiEvilTwin.getTargetSsid();
    html += "</strong> · ";
    html += String(WifiEvilTwin.getRemainingSec());
    html += " s left</div>";
  }
#endif
  html += "<div class='logo'><span class='w'>Hack</span><span class='r'>Card</span></div>";
  html += "<div class='avatar'>";
  html += c.ownerName[0] ? String(c.ownerName[0]) : "?";
  html += "</div>";
  html += "<h1>";
  html += c.ownerName;
  html += "</h1><p class='title'>";
  html += c.title;
  if (c.company[0] != '\0') {
    html += " · ";
    html += c.company;
  }
  html += "</p>";
  if (c.bio[0] != '\0') {
    html += "<p class='bio'>";
    html += c.bio;
    html += "</p>";
  }
  html += "<a class='btn' href='/profile.vcf'>Save Contact (vCard)</a>";
  if (c.email[0] != '\0') {
    html += "<a class='btn secondary' href='mailto:";
    html += c.email;
    html += "'>Email Me</a>";
  }
  if (c.phone[0] != '\0') {
    html += "<a class='btn secondary' href='tel:";
    html += c.phone;
    html += "'>Call</a>";
  }
  html += "<div class='links'>";
  if (c.website[0] != '\0') {
    html += "<a href='";
    html += c.website;
    html += "' target='_blank'>Website</a>";
  }
  if (c.linkedin[0] != '\0') {
    html += "<a href='";
    html += c.linkedin;
    html += "' target='_blank'>LinkedIn</a>";
  }
  if (c.github[0] != '\0') {
    html += "<a href='";
    html += c.github;
    html += "' target='_blank'>GitHub</a>";
  }
  html += "</div>";
  html += "<p class='foot' style='color:#6e7681'>Professional contact card</p>";
  (void)ip;
  html += "</div></body></html>";
  return html;
}

String WebStatusServer::buildTrainingRevealPage() const {
#if APP_WIFI_TRAINING
  return WifiTraining.buildRevealHtml(WifiAp.getIp());
#else
  return "<html><body>Training portal disabled</body></html>";
#endif
}


void WebStatusServer::handleNfcStatusApi() {
#if APP_NFC_READ
  NfcRead.ensureReady();
  String json = "{\"chip_ready\":";
  json += Nfc.isReady() ? "true" : "false";
  json += ",\"ready\":";
  json += Nfc.isReady() ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"history_sd_only\":";
  json += NfcHist.historySdOnly() ? "true" : "false";
  json += ",\"dumps_sd_only\":";
  json += NfcHist.dumpsSdOnly() ? "true" : "false";
  json += ",\"chip_info\":\"";
  json += Nfc.getChipInfo();
  json += "\",\"role\":\"reader_writer\"";
  json += ",\"tag_emulation\":false";
  json += ",\"protocol\":\"ISO14443A (NFC-A)\"";
  json += ",\"supported_tags\":\"NTAG213/215/216, MIFARE Ultralight, ISO14443A Type 2 (NDEF), MIFARE Classic 1K/4K (lab)\"";
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"chip_ready\":false}");
#endif
}

void WebStatusServer::handleNfcReadApi() {
#if APP_NFC_READ
  if (sServer.hasArg("dumps_sd")) {
    const bool sd = sServer.arg("dumps_sd") != "0" && sServer.arg("dumps_sd") != "false";
    NfcHist.setDumpsSdOnly(sd);
  }
  const NfcReadResult result = NfcRead.readOnce();
  sServer.send(200, "application/json", NfcRead.toJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcInfoApi() {
#if APP_NFC_INFO
  const NfcInfoResult result = NfcInfo.readInfo();
  sServer.send(200, "application/json", NfcInfo.toJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcDumpApi() {
#if APP_NFC_DUMP
  if (sServer.hasArg("dumps_sd")) {
    const bool sd = sServer.arg("dumps_sd") != "0" && sServer.arg("dumps_sd") != "false";
    NfcHist.setDumpsSdOnly(sd);
  }
  const NfcDumpResult result = NfcDump.dumpOnce();
  sServer.send(200, "application/json", NfcDump.toJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcWriteApi() {
#if APP_NFC_WRITE
  const String type = sServer.hasArg("type") ? sServer.arg("type") : "url";
  NfcWriteResult result;

  if (type == "text") {
    result = NfcWrite.writeText(sServer.arg("text").c_str());
  } else if (type == "vcard") {
    if (_ctx) {
      result = NfcWrite.writeVcard(_ctx->config);
    } else {
      strncpy(result.error, "Config not loaded", sizeof(result.error) - 1);
    }
  } else {
    result = NfcWrite.writeUri(sServer.arg("url").c_str());
  }

  sServer.send(200, "application/json", NfcWrite.writeJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcEraseApi() {
#if APP_NFC_WRITE
  const String pin = sServer.arg("pin");
  const NfcEraseResult result = NfcWrite.eraseTag(pin.c_str());
  sServer.send(200, "application/json", NfcWrite.eraseJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcHistoryApi() {
#if APP_NFC_READ
  sServer.send(200, "text/plain", NfcHist.readRecent(25));
#else
  sServer.send(503, "text/plain", "NFC disabled\n");
#endif
}

void WebStatusServer::handleNfcLastDumpApi() {
#if APP_NFC_READ
  const String path = NfcHist.lastDumpPath();
  if (path.isEmpty()) {
    sServer.send(200, "application/json", "{\"path\":\"\",\"ready\":false}");
    return;
  }
  String json = "{\"path\":\"";
  json += path;
  json += "\",\"ready\":true}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"path\":\"\",\"ready\":false}");
#endif
}

void WebStatusServer::handleNfcCloneApi() {
#if APP_NFC_CLONE
  const String pin = sServer.arg("pin");
  const String path = sServer.hasArg("path") ? sServer.arg("path") : "last";
  const NfcCloneResult result = NfcClone.restoreDump(path.c_str(), pin.c_str(), 4);
  sServer.send(200, "application/json", NfcClone.toJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcKillApi() {
#if APP_NFC_KILL
  const String pin = sServer.arg("pin");
  const NfcKillResult result = NfcKill.killTag(pin.c_str());
  sServer.send(200, "application/json", NfcKill.toJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcClassicDumpApi() {
#if APP_NFC_CLASSIC
  if (sServer.hasArg("dumps_sd")) {
    const bool sd = sServer.arg("dumps_sd") != "0" && sServer.arg("dumps_sd") != "false";
    NfcHist.setDumpsSdOnly(sd);
  }
  const String key = sServer.arg("key");
  const NfcClassicResult result = NfcClassic.dumpWithKey(key.c_str());
  sServer.send(200, "application/json", NfcClassic.dumpToJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcClassicCloneApi() {
#if APP_NFC_CLASSIC
  const String pin = sServer.arg("pin");
  const String key = sServer.arg("key");
  const NfcClassicResult result = NfcClassic.cloneLast(key.c_str(), pin.c_str());
  sServer.send(200, "application/json", NfcClassic.dumpToJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcKeysScanApi() {
#if APP_NFC_CLASSIC
  const uint8_t sector = sServer.hasArg("sector") ? static_cast<uint8_t>(sServer.arg("sector").toInt()) : 0;
  const NfcKeyScanResult result = NfcClassic.scanKey(sector);
  sServer.send(200, "application/json", NfcClassic.keyScanToJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcStoragePrefsApi() {
#if APP_NFC_READ
  if (sServer.hasArg("history_sd")) {
    const bool sd = sServer.arg("history_sd") != "0" && sServer.arg("history_sd") != "false";
    NfcHist.setHistorySdOnly(sd);
  }
  if (sServer.hasArg("dumps_sd")) {
    const bool sd = sServer.arg("dumps_sd") != "0" && sServer.arg("dumps_sd") != "false";
    NfcHist.setDumpsSdOnly(sd);
  }
  String json = "{\"success\":true,\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"history_sd_only\":";
  json += NfcHist.historySdOnly() ? "true" : "false";
  json += ",\"dumps_sd_only\":";
  json += NfcHist.dumpsSdOnly() ? "true" : "false";
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleNfcHistoryExportSdApi() {
#if APP_NFC_READ
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const bool ok = NfcHist.exportHistoryToSd(path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"NFC disabled\"}");
#endif
}

void WebStatusServer::handleNfcDumpExportSdApi() {
#if APP_NFC_DUMP
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const bool ok = NfcHist.exportLastDumpToSd(path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"NFC dump disabled\"}");
#endif
}

void WebStatusServer::handleNfcClassicExportSdApi() {
#if APP_NFC_CLASSIC
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const String src = NfcClassic.lastSavedPath();
  const bool ok = NfcHist.exportPathCopyToSd(src.c_str(), "classic", path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"NFC Classic disabled\"}");
#endif
}

void WebStatusServer::handleWifiScanApi() {
#if APP_WIFI_SCANNER
  if (WifiScanner.isScanning()) {
    sServer.send(200, "application/json", "{\"started\":true,\"scanning\":true}");
    return;
  }
  if (!WifiScanner.requestAsyncScan()) {
    sServer.send(500, "application/json", "{\"started\":false,\"success\":false,\"error\":\"Could not start scan\"}");
    return;
  }
  sServer.send(200, "application/json", "{\"started\":true,\"scanning\":true}");
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiScanClearApi() {
#if APP_WIFI_SCANNER
  if (WifiScanner.isScanning()) {
    sServer.send(409, "application/json", "{\"success\":false,\"error\":\"Scan in progress\"}");
    return;
  }
  if (!WifiScanner.clearLastScan()) {
    sServer.send(409, "application/json", "{\"success\":false,\"error\":\"Scan in progress — wait and try again\"}");
    return;
  }
  sServer.send(200, "application/json",
                "{\"success\":true,\"ready\":false,\"count\":0,\"networks\":[],\"cleared\":true}");
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiLastApi() {
#if APP_WIFI_SCANNER
  sServer.send(200, "application/json", WifiScanner.lastScanApiJson());
#else
  sServer.send(503, "application/json", "{\"ready\":false,\"success\":false}");
#endif
}

void WebStatusServer::handleWifiApGetApi() {
#if APP_WIFI_AP_CONTROL
  if (!_ctx) {
    sServer.send(500, "application/json", "{\"error\":\"not ready\"}");
    return;
  }
  const WifiApStatus status = WifiApControl.getStatus(_ctx->config);
  sServer.send(200, "application/json", WifiApControl.statusToJson(status));
#else
  sServer.send(503, "application/json", "{\"error\":\"disabled\"}");
#endif
}

void WebStatusServer::handleWifiApPostApi() {
#if APP_WIFI_AP_CONTROL
  if (!_ctx) {
    sServer.send(500, "application/json", "{\"success\":false,\"error\":\"not ready\"}");
    return;
  }

  if (!sServer.hasArg("ssid")) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"SSID required\"}");
    return;
  }

  const String ssid = sServer.arg("ssid");
  String password = sServer.hasArg("password") ? sServer.arg("password") : "";
  if (password.isEmpty()) {
    password = _ctx->config.apPassword;
  }
  const uint8_t channel =
      sServer.hasArg("channel") ? static_cast<uint8_t>(sServer.arg("channel").toInt()) : _ctx->config.apChannel;

  const WifiApApplyResult result = WifiApControl.apply(*_ctx, ssid.c_str(), password.c_str(), channel);
  const WifiApStatus status = WifiApControl.getStatus(_ctx->config);
  sServer.send(200, "application/json", WifiApControl.applyToJson(result, status));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiBeaconStatusApi() {
#if APP_WIFI_BEACON
  sServer.send(200, "application/json", WifiBeacon.statusToJson());
#else
  sServer.send(503, "application/json", "{\"running\":false}");
#endif
}

void WebStatusServer::handleWifiBeaconStartApi() {
#if APP_WIFI_BEACON
  const String pin = sServer.arg("pin");
  const String ssids = sServer.arg("ssids");
  const WifiBeaconResult result = WifiBeacon.start(pin.c_str(), ssids.c_str());
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"running\":";
  json += result.running ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiBeacon.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiBeaconStopApi() {
#if APP_WIFI_BEACON
  const WifiBeaconResult result = WifiBeacon.stop();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"running\":false,\"status\":";
  json += WifiBeacon.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTwinStatusApi() {
#if APP_WIFI_EVIL_TWIN
  sServer.send(200, "application/json", WifiEvilTwin.statusToJson());
#else
  sServer.send(503, "application/json", "{\"running\":false}");
#endif
}

void WebStatusServer::handleWifiTwinStartApi() {
#if APP_WIFI_EVIL_TWIN
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false,\"error\":\"Not ready\"}");
    return;
  }
  const String pin = sServer.arg("pin");
  const String ssid = sServer.arg("ssid");
  const uint8_t channel = sServer.hasArg("channel") ? static_cast<uint8_t>(sServer.arg("channel").toInt()) : 0;
  const uint8_t minutes = sServer.hasArg("minutes") ? static_cast<uint8_t>(sServer.arg("minutes").toInt()) : 5;
  const WifiEvilTwinResult result = WifiEvilTwin.start(*_ctx, pin.c_str(), ssid.c_str(), channel, minutes);
  bool portalStarted = false;
#if APP_WIFI_TRAINING
  if (result.success && sServer.hasArg("with_portal") && sServer.arg("with_portal") == "1") {
    const String templateArg = sServer.hasArg("template") ? sServer.arg("template") : "";
    if (templateArg.isEmpty() || templateArg == "auto") {
      WifiTraining.applyAutoTemplateForSsid(ssid.c_str());
    } else {
      WifiTraining.setTemplate(static_cast<uint8_t>(templateArg.toInt()));
      if (sServer.hasArg("network") && !sServer.arg("network").isEmpty()) {
        WifiTraining.setNetworkName(sServer.arg("network").c_str());
      } else if (!ssid.isEmpty()) {
        WifiTraining.setNetworkName(ssid.c_str());
      }
    }
    const WifiTrainingResult train = WifiTraining.start(pin.c_str());
    portalStarted = train.success;
  }
#else
  (void)portalStarted;
#endif
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"running\":";
  json += result.running ? "true" : "false";
  json += ",\"portal_started\":";
  json += portalStarted ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiEvilTwin.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTwinStopApi() {
#if APP_WIFI_EVIL_TWIN
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false}");
    return;
  }
  const WifiEvilTwinResult result = WifiEvilTwin.stop(*_ctx);
#if APP_WIFI_TRAINING
  if (WifiTraining.isActive()) {
    WifiTraining.stop();
  }
#endif
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"running\":false,\"status\":";
  json += WifiEvilTwin.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingStatusApi() {
#if APP_WIFI_TRAINING
  sServer.send(200, "application/json", WifiTraining.statusToJson());
#else
  sServer.send(503, "application/json", "{\"active\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingConfigApi() {
#if APP_WIFI_TRAINING
  const uint8_t templateId = sServer.hasArg("template") ? static_cast<uint8_t>(sServer.arg("template").toInt()) : 0;
  const String network = sServer.arg("network");
  const String headline = sServer.arg("headline");
  const String subtitle = sServer.arg("subtitle");

  bool ok = WifiTraining.setTemplate(templateId);
  if (network.length() > 0) {
    ok = WifiTraining.setNetworkName(network.c_str()) && ok;
  }
  if (headline.length() > 0) {
    ok = WifiTraining.setHeadline(headline.c_str()) && ok;
  }
  if (subtitle.length() > 0) {
    ok = WifiTraining.setSubtitle(subtitle.c_str()) && ok;
  }

  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"config\":";
  json += WifiTraining.configToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingStartApi() {
#if APP_WIFI_TRAINING
  const String pin = sServer.arg("pin");
  const WifiTrainingResult result = WifiTraining.start(pin.c_str());
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"active\":";
  json += result.active ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiTraining.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingStopApi() {
#if APP_WIFI_TRAINING
  const WifiTrainingResult result = WifiTraining.stop();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"active\":false,\"status\":";
  json += WifiTraining.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingSubmitApi() {
#if APP_WIFI_TRAINING
  if (!WifiTraining.isActive()) {
    sServer.send(403, "text/plain", "Training portal not active");
    return;
  }
  String username = sServer.arg("username");
  String password = sServer.arg("password");
  username.trim();
  password.trim();
  if (username.isEmpty() && password.isEmpty() && sServer.hasArg("plain")) {
    const String body = sServer.arg("plain");
    int uStart = body.indexOf("username=");
    if (uStart >= 0) {
      uStart += 9;
      int uEnd = body.indexOf('&', uStart);
      username = (uEnd < 0) ? body.substring(uStart) : body.substring(uStart, uEnd);
      username.replace("+", " ");
    }
    int pStart = body.indexOf("password=");
    if (pStart >= 0) {
      pStart += 9;
      int pEnd = body.indexOf('&', pStart);
      password = (pEnd < 0) ? body.substring(pStart) : body.substring(pStart, pEnd);
      password.replace("+", " ");
    }
  }
  if (!WifiTraining.recordCapture(username.c_str(), password.c_str())) {
    Log.warn("wifi", "training submit: capture not persisted");
  }
  sServer.sendHeader("Location", String("http://") + WifiAp.getIp() + "/portal/reveal", true);
  sServer.send(302, "text/plain", "Redirecting to training reveal");
#else
  sServer.send(503, "text/plain", "Training portal disabled");
#endif
}

void WebStatusServer::handleWifiTrainingCapturesApi() {
#if APP_WIFI_TRAINING
  sServer.send(200, "text/plain", WifiTraining.getCapturesTail());
#else
  sServer.send(503, "text/plain", "Training portal disabled");
#endif
}

void WebStatusServer::handleWifiTrainingCapturesClearApi() {
#if APP_WIFI_TRAINING
  const bool ok = WifiTraining.clearCaptures();
  sServer.send(ok ? 200 : 500, "text/plain", ok ? "Captures cleared\n" : "Could not clear\n");
#else
  sServer.send(503, "text/plain", "Training portal disabled");
#endif
}

void WebStatusServer::handleWifiMonitorStatusApi() {
#if APP_WIFI_MONITOR
  sServer.send(200, "application/json", WifiMonitor.statusToJson());
#else
  sServer.send(503, "application/json", "{\"sniffing\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorProbesApi() {
#if APP_WIFI_MONITOR
  sServer.send(200, "application/json", WifiMonitor.probesToJson());
#else
  sServer.send(503, "application/json", "{\"probes\":[]}");
#endif
}

void WebStatusServer::handleWifiMonitorSniffStartApi() {
#if APP_WIFI_MONITOR
  const String pin = sServer.arg("pin");
  const uint8_t minutes = sServer.hasArg("minutes") ? static_cast<uint8_t>(sServer.arg("minutes").toInt()) : 3;
  const bool saveSd = sServer.hasArg("save_sd") && sServer.arg("save_sd") != "0" && sServer.arg("save_sd") != "false";
  const WifiMonitorResult result = WifiMonitor.startSniff(pin.c_str(), minutes, saveSd);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiMonitor.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorSniffStopApi() {
#if APP_WIFI_MONITOR
  const WifiMonitorResult result = WifiMonitor.stopSniff();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"status\":";
  json += WifiMonitor.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorDeauthStartApi() {
#if APP_WIFI_MONITOR
  const String pin = sServer.arg("pin");
  const String bssid = sServer.arg("bssid");
  const uint8_t channel = sServer.hasArg("channel") ? static_cast<uint8_t>(sServer.arg("channel").toInt()) : 0;
  const uint8_t count = sServer.hasArg("count") ? static_cast<uint8_t>(sServer.arg("count").toInt()) : 5;
  const uint8_t attackMode = sServer.hasArg("mode") ? static_cast<uint8_t>(sServer.arg("mode").toInt()) : 0;
  const WifiMonitorResult result = WifiMonitor.startDeauth(pin.c_str(), bssid.c_str(), channel, count, attackMode);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiMonitor.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorDeauthStopApi() {
#if APP_WIFI_MONITOR
  const WifiMonitorResult result = WifiMonitor.stopDeauth();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"status\":";
  json += WifiMonitor.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorProbesClearApi() {
#if APP_WIFI_MONITOR
  const bool ok = WifiMonitor.clearProbes();
  sServer.send(ok ? 200 : 500, "application/json", ok ? "{\"success\":true}" : "{\"success\":false}");
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiMonitorProbesExportSdApi() {
#if APP_WIFI_MONITOR
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const bool ok = WifiMonitor.exportProbeLogToSd(path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"Monitor disabled\"}");
#endif
}

void WebStatusServer::handleBleStatusApi() {
#if APP_BLE
  String json = BleLab.statusToJson();
  json.remove(json.length() - 1);
  json += ",";
  json += BleContactCard.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"ble_ready\":false}");
#endif
}

void WebStatusServer::handleBleScanApi() {
#if APP_BLE
  const BleScanResult result = BleLab.scanOnce(5);
  sServer.send(200, "application/json", BleLab.scanToJson(result));
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleLastScanApi() {
#if APP_BLE
  sServer.send(200, "application/json", BleLab.readLastScanJson());
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleAdvertiseStartApi() {
#if APP_BLE
  const String pin = sServer.arg("pin");
  const String name = sServer.arg("name");
  const BleAdvertiseResult result = BleLab.startAdvertise(pin.c_str(), name.c_str());
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"advertising\":";
  json += result.advertising ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += BleLab.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleAdvertiseStopApi() {
#if APP_BLE
  const BleAdvertiseResult result = BleLab.stopAdvertise();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"advertising\":false,\"status\":";
  json += BleLab.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleContactStartApi() {
#if APP_BLE
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false,\"error\":\"Not ready\"}");
    return;
  }
  const BleContactCardResult result = BleContactCard.start(_ctx->config);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"active\":";
  json += result.active ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleContactStopApi() {
#if APP_BLE
  const BleContactCardResult result = BleContactCard.stop();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"active\":false}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleBleContactRefreshApi() {
#if APP_BLE
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false}");
    return;
  }
  const BleContactCardResult result = BleContactCard.refresh(_ctx->config);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleHidDeviceApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "text/plain", "Context not ready\n");
    return;
  }

  if (!sServer.hasArg("ap_ssid")) {
    sServer.send(400, "text/plain", "Missing ap_ssid\n");
    return;
  }

  const String ssid = sServer.arg("ap_ssid");
  if (ssid.length() == 0 || ssid.length() > 31) {
    sServer.send(400, "text/plain", "SSID must be 1–31 characters\n");
    return;
  }

  String password = sServer.hasArg("ap_password") ? sServer.arg("ap_password") : _ctx->config.apPassword;
  if (password.length() > 0 && password.length() < 8) {
    sServer.send(400, "text/plain", "Wi-Fi password must be at least 8 characters\n");
    return;
  }

  if (sServer.hasArg("lab_pin")) {
    String labPin = sServer.arg("lab_pin");
    labPin.trim();
    if (labPin.length() < 4 || labPin.length() > 15) {
      sServer.send(400, "text/plain", "Lab PIN must be 4–15 characters\n");
      return;
    }
    strlcpy(_ctx->config.labModePin, labPin.c_str(), sizeof(_ctx->config.labModePin));
  }

#if APP_WIFI_AP_CONTROL
  WifiApControl.apply(*_ctx, ssid.c_str(), password.c_str(), _ctx->config.apChannel);
#else
  strlcpy(_ctx->config.apSsid, ssid.c_str(), sizeof(_ctx->config.apSsid));
  strlcpy(_ctx->config.apPassword, password.c_str(), sizeof(_ctx->config.apPassword));
  WifiAp.begin(_ctx->config.apSsid, _ctx->config.apPassword, _ctx->config.apChannel);
#endif

  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Applied but could not save to flash\n");
    return;
  }

  sServer.send(200, "text/plain", "Device settings saved. Rejoin Wi-Fi if SSID/password changed.\n");
#else
  sServer.send(503, "text/plain", "USB HID not enabled\n");
#endif
}

void WebStatusServer::handleHidStatusApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"ready\":false}");
    return;
  }
  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  sServer.send(200, "application/json", HidLab.statusToJson(os));
#else
  sServer.send(503, "application/json", "{\"ready\":false}");
#endif
}

void WebStatusServer::handleHidOsApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "text/plain", "Context not ready\n");
    return;
  }
  if (!sServer.hasArg("os")) {
    sServer.send(400, "text/plain", "Missing os\n");
    return;
  }
  const int osVal = sServer.arg("os").toInt();
  if (osVal < 0 || osVal >= static_cast<int>(HidTargetOs::Count)) {
    sServer.send(400, "text/plain", "Invalid OS (0=Windows, 1=macOS, 2=Linux)\n");
    return;
  }
  const HidTargetOs os = static_cast<HidTargetOs>(osVal);
  if (!HidLab.setTargetOs(_ctx->config, os)) {
    sServer.send(400, "text/plain", "Invalid OS\n");
    return;
  }
  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Failed to save config\n");
    return;
  }
  sServer.send(200, "text/plain", String("Target OS set to ") + hidOsName(os) + "\n");
#else
  sServer.send(503, "text/plain", "USB HID not enabled\n");
#endif
}

void WebStatusServer::handleHidRunApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "text/plain", "Context not ready\n");
    return;
  }
  if (!sServer.hasArg("action")) {
    sServer.send(400, "text/plain", "Missing action\n");
    return;
  }

  String pinArg = sServer.hasArg("pin") ? sServer.arg("pin") : "";
  pinArg.trim();
  if (pinArg.isEmpty()) {
    sServer.send(400, "text/plain", "Lab PIN required — enter your PIN first\n");
    return;
  }

  String action = sServer.arg("action");
  action.trim();

  String payload;
  if (action == "type_text") {
    payload = sServer.hasArg("text") ? sServer.arg("text") : "";
  } else if (action == "open_website") {
    payload = sServer.hasArg("url") ? sServer.arg("url") : "";
    payload.trim();
  } else if (action == "key_press") {
    payload = sServer.hasArg("key") ? sServer.arg("key") : "";
    payload.trim();
  } else if (action == "mouse_click") {
    payload = sServer.hasArg("button") ? sServer.arg("button") : "left";
    payload.trim();
  } else if (action == "mouse_move") {
    if (sServer.hasArg("dx") && sServer.hasArg("dy")) {
      payload = sServer.arg("dx") + String(",") + sServer.arg("dy");
    }
  } else if (action == "unlock_screen") {
    payload = sServer.hasArg("password") ? sServer.arg("password") : "";
  }

  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  const HidLabResult result = HidLab.runAction(os, action.c_str(), pinArg.c_str(), payload.c_str());
  if (!result.success) {
    sServer.send(403, "text/plain", String(result.error) + "\n");
    return;
  }
  sServer.send(200, "text/plain", String(result.error) + "\n");
#else
  sServer.send(503, "text/plain", "USB HID not enabled\n");
#endif
}

void WebStatusServer::handleHidPayloadsApi() {
#if APP_USB_HID
  sServer.send(200, "application/json", HidPayloads.listToJson());
#else
  sServer.send(503, "application/json", "{\"count\":0,\"payloads\":[]}");
#endif
}

void WebStatusServer::handleHidPayloadReadApi() {
#if APP_USB_HID
  const String name = sServer.arg("name");
  if (name.isEmpty()) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"Missing name\"}");
    return;
  }
  String script;
  if (!HidPayloads.readPayload(name.c_str(), script)) {
    sServer.send(404, "application/json", "{\"success\":false,\"error\":\"Not found\"}");
    return;
  }
  String json = "{\"success\":true,\"name\":\"" + name + "\",\"content\":\"";
  for (size_t i = 0; i < script.length(); i++) {
    const char c = script.charAt(i);
    if (c == '\\' || c == '"') {
      json += '\\';
    }
    if (c == '\n') {
      json += "\\n";
    } else if (c == '\r') {
      continue;
    } else {
      json += c;
    }
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

namespace {

void readHidPayloadSaveFields(String& pin, String& name, String& content) {
  pin = sServer.arg("pin");
  name = sServer.arg("name");
  pin.trim();
  name.trim();
  if (sServer.hasArg("plain")) {
    content = sServer.arg("plain");
  } else {
    content = sServer.arg("content");
  }
}

}  // namespace

void WebStatusServer::handleHidPayloadSaveApi() {
#if APP_USB_HID
  String pin;
  String name;
  String content;
  readHidPayloadSaveFields(pin, name, content);
  char pinError[64] = {0};
  if (!labPinCheck(pin.c_str(), pinError, sizeof(pinError))) {
    String json = "{\"success\":false,\"error\":\"";
    json += pinError;
    json += "\"}";
    sServer.send(403, "application/json", json);
    return;
  }
  if (name.isEmpty()) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"Filename required (.duck or .txt)\"}");
    return;
  }
  if (content.isEmpty()) {
    sServer.send(400, "application/json",
                 "{\"success\":false,\"error\":\"Script body missing — use Save to device again (sends editor text as raw body)\"}");
    return;
  }
  char writeErr[160] = {0};
  if (!HidPayloads.writePayload(name.c_str(), content, writeErr, sizeof(writeErr))) {
    String json = "{\"success\":false,\"flash_free_bytes\":";
    json += String(static_cast<unsigned>(Storage.freeFlashBytes()));
    json += ",\"error\":\"";
    json += writeErr[0] ? writeErr : "Save failed";
    json += "\"}";
    sServer.send(500, "application/json", json);
    return;
  }
  String json = "{\"success\":true,\"name\":\"";
  json += name;
  json += "\",\"size\":";
  json += String(content.length());
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleHidPayloadSaveSdApi() {
#if APP_USB_HID
  String pin;
  String name;
  String content;
  readHidPayloadSaveFields(pin, name, content);
  char pinError[64] = {0};
  if (!labPinCheck(pin.c_str(), pinError, sizeof(pinError))) {
    String json = "{\"success\":false,\"sd_ready\":";
    json += Storage.isSdReady() ? "true" : "false";
    json += ",\"error\":\"";
    json += pinError;
    json += "\"}";
    sServer.send(403, "application/json", json);
    return;
  }
  if (respondSdExportUnavailable()) {
    return;
  }
  if (name.isEmpty() || content.isEmpty()) {
    sServer.send(400, "application/json",
                 "{\"success\":false,\"sd_ready\":true,\"error\":\"name and content required\"}");
    return;
  }
  if (!HidPayloads.writePayloadSdOnly(name.c_str(), content)) {
    sServer.send(500, "application/json",
                 "{\"success\":false,\"sd_ready\":true,\"error\":\"Save to SD failed — check name (.duck/.txt) and size (max 4KB)\"}");
    return;
  }
  String json = "{\"success\":true,\"sd_ready\":true,\"name\":\"";
  json += name;
  json += "\",\"size\":";
  json += String(content.length());
  json += ",\"path\":\"/payloads/";
  json += name;
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false}");
#endif
}

void WebStatusServer::handleHidPayloadDeleteApi() {
#if APP_USB_HID
  const String pin = sServer.arg("pin");
  const String name = sServer.arg("name");
  char pinError[64] = {0};
  if (!labPinCheck(pin.c_str(), pinError, sizeof(pinError))) {
    String json = "{\"success\":false,\"error\":\"";
    json += pinError;
    json += "\"}";
    sServer.send(403, "application/json", json);
    return;
  }
  if (name.isEmpty()) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"name required\"}");
    return;
  }
  if (!HidPayloads.deletePayload(name.c_str())) {
    sServer.send(404, "application/json", "{\"success\":false,\"error\":\"Not found or delete failed\"}");
    return;
  }
  sServer.send(200, "application/json", "{\"success\":true}");
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleHidRunScriptApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false,\"error\":\"Not ready\"}");
    return;
  }
  const String pin = sServer.arg("pin");
  String name = sServer.arg("name");
  name.trim();
  if (pin.isEmpty()) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"Lab PIN required\"}");
    return;
  }
  HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  if (sServer.hasArg("os")) {
    os = hidOsFromString(sServer.arg("os").c_str());
  }

  HidLabResult result;
  const String inlineScript = sServer.hasArg("plain") ? sServer.arg("plain") : String();
  if (inlineScript.length() > 0) {
    result = HidLab.runScriptContent(pin.c_str(), inlineScript.c_str(), os);
  } else if (!name.isEmpty()) {
    result = HidLab.runPayloadScript(pin.c_str(), name.c_str(), os);
  } else {
    sServer.send(400, "application/json",
                 "{\"success\":false,\"error\":\"Send script in request body or provide saved payload name\"}");
    return;
  }
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += HidLab.statusToJson(os);
  json += "}";
  sServer.send(result.success ? 200 : 403, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleHidArmApi() {
#if APP_USB_HID
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false}");
    return;
  }
  if (sServer.hasArg("disarm") && sServer.arg("disarm") == "1") {
    HidLab.disarmButtonPayload();
    sServer.send(200, "application/json", "{\"success\":true,\"button_armed\":false}");
    return;
  }
  const String pin = sServer.arg("pin");
  const String name = sServer.arg("name");
  if (pin.isEmpty() || name.isEmpty()) {
    sServer.send(400, "application/json", "{\"success\":false,\"error\":\"pin and name required\"}");
    return;
  }
  HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  if (sServer.hasArg("os")) {
    os = hidOsFromString(sServer.arg("os").c_str());
  }
  const HidLabResult result = HidLab.armButtonPayload(pin.c_str(), name.c_str(), os);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",";
  json += HidLab.armStatusToJson().substring(1);
  sServer.send(result.success ? 200 : 403, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleCaptiveRedirect() {
  const String target = String("http://") + WifiAp.getIp() + "/portal";
  sServer.sendHeader("Location", target, true);
  sServer.send(302, "text/plain", "Redirecting to HackCard portal");
}

void WebStatusServer::handleCaptiveDetect() {
  if (_ctx && (_ctx->config.captivePortalEnabled
#if APP_WIFI_TRAINING
               || WifiTraining.isActive()
#endif
               )) {
    handleCaptiveRedirect();
    return;
  }
  sServer.send(204, "text/plain", "");
}

void WebStatusServer::handleNotFound() {
#if APP_WEB_UI
  if (tryServeWwwStatic(sServer.uri())) {
    return;
  }
#endif
  if (_ctx && (_ctx->config.captivePortalEnabled
#if APP_WIFI_TRAINING
               || WifiTraining.isActive()
#endif
               )) {
    handleCaptiveRedirect();
    return;
  }
  sServer.send(404, "text/plain", "Not found\n");
}

void WebStatusServer::handleLogsApi() {
#if APP_SYSTEM_LOGS
  String logs;
  ActivityLogs.readTail(logs, 50);
  sServer.send(200, "text/plain", logs);
#else
  sServer.send(503, "text/plain", "Logs disabled\n");
#endif
}

void WebStatusServer::handleLogsClearApi() {
#if APP_SYSTEM_LOGS
  if (ActivityLogs.clear()) {
    Log.activity("logs", "clear", "ok");
    sServer.send(200, "text/plain", "Logs cleared\n");
  } else {
    sServer.send(500, "text/plain", "Could not clear logs\n");
  }
#else
  sServer.send(503, "text/plain", "Logs disabled\n");
#endif
}

String WebStatusServer::getCookieValue(const char* name) const {
  if (!name || !sServer.hasHeader("Cookie")) {
    return "";
  }
  const String cookies = sServer.header("Cookie");
  const String prefix = String(name) + "=";
  const int idx = cookies.indexOf(prefix);
  if (idx < 0) {
    return "";
  }
  const int start = idx + prefix.length();
  int end = cookies.indexOf(';', start);
  if (end < 0) {
    end = cookies.length();
  }
  String value = cookies.substring(start, end);
  value.trim();
  return value;
}

bool WebStatusServer::validateLabSessionToken(const String& token) const {
  return token.length() > 0 && token == _labSessionToken && millis() < _labSessionExpiryMs;
}

void WebStatusServer::grantLabSession() {
  _labSessionToken = String(esp_random(), HEX) + String(esp_random(), HEX);
  _labSessionExpiryMs = millis() + 3600000UL;
}

void WebStatusServer::persistLabSessionCookie() {
  if (_labSessionToken.length() == 0) {
    return;
  }
  sServer.sendHeader("Set-Cookie",
                     "hackcard_lab=" + _labSessionToken + "; Path=/; Max-Age=3600; SameSite=Lax");
}

bool WebStatusServer::hasLabSession() const {
  if (_labSessionToken.length() == 0 || millis() >= _labSessionExpiryMs) {
    return false;
  }
  if (sServer.hasArg("st") && validateLabSessionToken(sServer.arg("st"))) {
    return true;
  }
  return validateLabSessionToken(getCookieValue("hackcard_lab"));
}

bool WebStatusServer::hasLabAccess() const {
  if (!_ctx) {
    return false;
  }
  if (DeviceModeManager::isLabMode(*_ctx)) {
    return true;
  }
  return hasLabSession();
}

void WebStatusServer::handleBusinessProfile() {
#if APP_WIFI_TRAINING
  if (WifiTraining.isActive()) {
    sServer.send(200, "text/html", WifiTraining.buildPortalHtml(WifiAp.getIp()));
    return;
  }
#endif
#if APP_WEB_UI
  if (tryServeWwwStatic("/profile.html")) {
    return;
  }
#endif
  sServer.send(200, "text/html", buildPortalPage());
}

void WebStatusServer::handleLabConsoleUi() {
#if APP_WEB_UI
  if (_ctx && DeviceModeManager::isBusinessMode(*_ctx) && !hasLabAccess()) {
    sServer.sendHeader("Set-Cookie", "hackcard_lab=; Path=/; Max-Age=0; SameSite=Lax");
    sServer.sendHeader("Location", "/portal", true);
    sServer.send(302, "text/plain", "Lab access required\n");
    return;
  }
  persistLabSessionCookie();
  handleDashboardUi();
#endif
}

void WebStatusServer::handleRootRoute() {
#if APP_WEB_UI
  if (_ctx && DeviceModeManager::isBusinessMode(*_ctx) && !hasLabAccess()) {
    handleBusinessProfile();
    return;
  }
#endif
  handleDashboardUi();
}

void WebStatusServer::handleLabUnlockApi() {
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }
  if (!sServer.hasArg("pin")) {
    sServer.send(400, "application/json", "{\"ok\":false,\"error\":\"Missing pin\"}");
    return;
  }

  char pinError[64] = {};
  const String pin = sServer.arg("pin");
  if (!labPinCheck(pin.c_str(), pinError, sizeof(pinError))) {
    sServer.send(403, "application/json", "{\"ok\":false,\"error\":\"Invalid lab password\"}");
    return;
  }

  grantLabSession();
  persistLabSessionCookie();
  String json = "{\"ok\":true,\"token\":\"";
  json += _labSessionToken;
  json += "\"}";
  sServer.send(200, "application/json", json);
}

void WebStatusServer::handleModesGetApi() {
#if APP_MODES
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }
  const DeviceMode mode = DeviceModeManager::fromStored(_ctx->config.deviceMode);
  sServer.send(200, "application/json", DeviceModeManager::listToJson(mode));
#else
  sServer.send(503, "text/plain", "Modes disabled\n");
#endif
}

void WebStatusServer::handleModeApi() {
#if APP_MODES
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }

  if (!sServer.hasArg("mode")) {
    sServer.send(400, "text/plain", "Missing mode parameter\n");
    return;
  }

  const DeviceMode mode = DeviceModeManager::fromString(sServer.arg("mode").c_str());
  DeviceModeManager::apply(*_ctx, mode);

  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Could not save mode\n");
    return;
  }

  sServer.send(200, "text/plain", "Mode activated!\n");
#else
  sServer.send(503, "text/plain", "Modes disabled\n");
#endif
}

void WebStatusServer::handleProfilePublicApi() {
  if (!_ctx) {
    sServer.send(500, "application/json", "{\"error\":\"not ready\"}");
    return;
  }

  const HackCardConfig& c = _ctx->config;
  auto esc = [](const char* s) -> String {
    String out;
    if (!s) {
      return out;
    }
    for (const char* p = s; *p; p++) {
      const char ch = *p;
      if (ch == '\\' || ch == '"') {
        out += '\\';
      }
      out += ch;
    }
    return out;
  };

  String json = "{";
  json += "\"name\":\"" + esc(c.ownerName) + "\",";
  json += "\"title\":\"" + esc(c.title) + "\",";
  json += "\"company\":\"" + esc(c.company) + "\",";
  json += "\"email\":\"" + esc(c.email) + "\",";
  json += "\"phone\":\"" + esc(c.phone) + "\",";
  json += "\"website\":\"" + esc(c.website) + "\",";
  json += "\"linkedin\":\"" + esc(c.linkedin) + "\",";
  json += "\"github\":\"" + esc(c.github) + "\",";
  json += "\"bio\":\"" + esc(c.bio) + "\"";
  json += "}";
  sServer.send(200, "application/json", json);
}

void WebStatusServer::handleProfileApi() {
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }

  if (DeviceModeManager::isBusinessMode(*_ctx) && !hasLabAccess()) {
    char pinError[64] = {};
    if (!sServer.hasArg("pin") || !labPinCheck(sServer.arg("pin").c_str(), pinError, sizeof(pinError))) {
      sServer.send(403, "text/plain", "Profile editing requires lab access (unlock lab or send lab PIN)\n");
      return;
    }
  }

  if (sServer.hasArg("name")) {
    strlcpy(_ctx->config.ownerName, sServer.arg("name").c_str(), sizeof(_ctx->config.ownerName));
  }
  if (sServer.hasArg("title")) {
    strlcpy(_ctx->config.title, sServer.arg("title").c_str(), sizeof(_ctx->config.title));
  }
  if (sServer.hasArg("company")) {
    strlcpy(_ctx->config.company, sServer.arg("company").c_str(), sizeof(_ctx->config.company));
  }
  if (sServer.hasArg("email")) {
    strlcpy(_ctx->config.email, sServer.arg("email").c_str(), sizeof(_ctx->config.email));
  }
  if (sServer.hasArg("phone")) {
    strlcpy(_ctx->config.phone, sServer.arg("phone").c_str(), sizeof(_ctx->config.phone));
  }
  if (sServer.hasArg("website")) {
    strlcpy(_ctx->config.website, sServer.arg("website").c_str(), sizeof(_ctx->config.website));
  }
  if (sServer.hasArg("linkedin")) {
    strlcpy(_ctx->config.linkedin, sServer.arg("linkedin").c_str(), sizeof(_ctx->config.linkedin));
  }
  if (sServer.hasArg("github")) {
    strlcpy(_ctx->config.github, sServer.arg("github").c_str(), sizeof(_ctx->config.github));
  }
  if (sServer.hasArg("bio")) {
    strlcpy(_ctx->config.bio, sServer.arg("bio").c_str(), sizeof(_ctx->config.bio));
  }

  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Could not save profile\n");
    return;
  }

#if APP_BLE
  if (BleContactCard.isActive()) {
    BleContactCard.refresh(_ctx->config);
  }
#endif

  sServer.send(200, "text/plain", "Profile saved!\n");
}

void WebStatusServer::handleSettingsApi() {
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }

  if (sServer.hasArg("brightness")) {
    int brightness = sServer.arg("brightness").toInt();
    if (brightness < 10) {
      brightness = 10;
    }
    if (brightness > 100) {
      brightness = 100;
    }
    _ctx->config.ringBrightness = static_cast<uint8_t>(brightness);
  }
  if (sServer.hasArg("buzzer_volume")) {
    int volume = sServer.arg("buzzer_volume").toInt();
    if (volume < 10) {
      volume = 10;
    }
    if (volume > 100) {
      volume = 100;
    }
    _ctx->config.buzzerVolume = static_cast<uint8_t>(volume);
  }
  if (sServer.hasArg("buzzer")) {
    _ctx->config.buzzerEnabled = sServer.arg("buzzer") == "1";
  }
  if (sServer.hasArg("bootanim")) {
    _ctx->config.bootAnimation = sServer.arg("bootanim") == "1";
  }
  if (sServer.hasArg("logging")) {
    _ctx->config.activityLogging = sServer.arg("logging") == "1";
  }
#if APP_WIFI_PORTAL
  if (sServer.hasArg("captive")) {
    _ctx->config.captivePortalEnabled = sServer.arg("captive") == "1";
    WifiCaptive.setEnabled(_ctx->config.captivePortalEnabled);
  }
#endif
  if (sServer.hasArg("tune_success")) {
    uint8_t tune = static_cast<uint8_t>(sServer.arg("tune_success").toInt());
    _ctx->config.tuneSuccess = tune >= static_cast<uint8_t>(BuzzerTuneId::Count) ? 0 : tune;
  }
  if (sServer.hasArg("tune_error")) {
    uint8_t tune = static_cast<uint8_t>(sServer.arg("tune_error").toInt());
    _ctx->config.tuneError = tune >= static_cast<uint8_t>(BuzzerTuneId::Count) ? 0 : tune;
  }
  if (sServer.hasArg("tune_boot")) {
    uint8_t tune = static_cast<uint8_t>(sServer.arg("tune_boot").toInt());
    _ctx->config.tuneBoot = tune >= static_cast<uint8_t>(BuzzerTuneId::Count) ? 0 : tune;
  }
  if (sServer.hasArg("tune_nfc")) {
    uint8_t tune = static_cast<uint8_t>(sServer.arg("tune_nfc").toInt());
    _ctx->config.tuneNfc = tune >= static_cast<uint8_t>(BuzzerTuneId::Count) ? 0 : tune;
  }
  if (sServer.hasArg("boot_ring_pattern")) {
    int pattern = sServer.arg("boot_ring_pattern").toInt();
    if (pattern < 0) {
      pattern = 0;
    }
    if (pattern > 12) {
      pattern = 12;
    }
    _ctx->config.bootRingPattern = static_cast<uint8_t>(pattern);
  }
  if (sServer.hasArg("boot_ring_r")) {
    _ctx->config.bootRingR = static_cast<uint8_t>(sServer.arg("boot_ring_r").toInt());
  }
  if (sServer.hasArg("boot_ring_g")) {
    _ctx->config.bootRingG = static_cast<uint8_t>(sServer.arg("boot_ring_g").toInt());
  }
  if (sServer.hasArg("boot_ring_b")) {
    _ctx->config.bootRingB = static_cast<uint8_t>(sServer.arg("boot_ring_b").toInt());
  }
  if (sServer.hasArg("wifi_led_pattern")) {
    int p = sServer.arg("wifi_led_pattern").toInt();
    if (p < 0) {
      p = 0;
    }
    if (p > 2) {
      p = 2;
    }
    _ctx->config.wifiLedPattern = static_cast<uint8_t>(p);
  }
  if (sServer.hasArg("wifi_led_r")) {
    _ctx->config.wifiLedR = static_cast<uint8_t>(sServer.arg("wifi_led_r").toInt());
  }
  if (sServer.hasArg("wifi_led_g")) {
    _ctx->config.wifiLedG = static_cast<uint8_t>(sServer.arg("wifi_led_g").toInt());
  }
  if (sServer.hasArg("wifi_led_b")) {
    _ctx->config.wifiLedB = static_cast<uint8_t>(sServer.arg("wifi_led_b").toInt());
  }
  if (sServer.hasArg("wifi_ring_pattern")) {
    int pattern = sServer.arg("wifi_ring_pattern").toInt();
    if (pattern < 0) {
      pattern = 0;
    }
    if (pattern > 12) {
      pattern = 12;
    }
    _ctx->config.wifiRingPattern = static_cast<uint8_t>(pattern);
  }
  if (sServer.hasArg("wifi_ring_r")) {
    _ctx->config.wifiRingR = static_cast<uint8_t>(sServer.arg("wifi_ring_r").toInt());
  }
  if (sServer.hasArg("wifi_ring_g")) {
    _ctx->config.wifiRingG = static_cast<uint8_t>(sServer.arg("wifi_ring_g").toInt());
  }
  if (sServer.hasArg("wifi_ring_b")) {
    _ctx->config.wifiRingB = static_cast<uint8_t>(sServer.arg("wifi_ring_b").toInt());
  }

  if (sServer.hasArg("lab_pin")) {
    String labPin = sServer.arg("lab_pin");
    labPin.trim();
    if (labPin.length() < 4 || labPin.length() > 15) {
      sServer.send(400, "text/plain", "Lab PIN must be 4–15 characters\n");
      return;
    }
    strlcpy(_ctx->config.labModePin, labPin.c_str(), sizeof(_ctx->config.labModePin));
  }

  bool apChanged = false;
  if (sServer.hasArg("ap_ssid")) {
    const String ssid = sServer.arg("ap_ssid");
    if (ssid.length() == 0 || ssid.length() > 31) {
      sServer.send(400, "text/plain", "SSID must be 1–31 characters\n");
      return;
    }
    String password = sServer.hasArg("ap_password") ? sServer.arg("ap_password") : String(_ctx->config.apPassword);
    if (password.length() > 0 && password.length() < 8) {
      sServer.send(400, "text/plain", "Wi-Fi password must be at least 8 characters\n");
      return;
    }
    apChanged = (ssid != String(_ctx->config.apSsid)) || (password != String(_ctx->config.apPassword));
    if (apChanged) {
#if APP_WIFI_AP_CONTROL
      WifiApControl.apply(*_ctx, ssid.c_str(), password.c_str(), _ctx->config.apChannel);
#else
      strlcpy(_ctx->config.apSsid, ssid.c_str(), sizeof(_ctx->config.apSsid));
      strlcpy(_ctx->config.apPassword, password.c_str(), sizeof(_ctx->config.apPassword));
      WifiAp.begin(_ctx->config.apSsid, _ctx->config.apPassword, _ctx->config.apChannel);
#endif
    }
  }

  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Could not save settings\n");
    return;
  }

  Log.begin(_ctx->config.activityLogging);
  LedBuzzer.applySettings(_ctx->config);
  if (apChanged) {
    sServer.send(200, "text/plain", "Settings saved. Rejoin Wi-Fi if SSID/password changed.\n");
  } else {
    sServer.send(200, "text/plain", "Settings saved and applied!\n");
  }
}

void WebStatusServer::handleAppsApi() {
  if (!sServer.hasArg("action")) {
    sServer.send(400, "text/plain", "Missing action\n");
    return;
  }

  const String action = sServer.arg("action");

#if APP_DEMO_LED_BUZZER
  if (action == "ring_brightness") {
    if (!_ctx || !sServer.hasArg("value")) {
      sServer.send(400, "text/plain", "Missing brightness value\n");
      return;
    }
    const uint8_t brightness = static_cast<uint8_t>(sServer.arg("value").toInt());
    if (!LedBuzzer.setRingBrightness(brightness, _ctx->config)) {
      sServer.send(503, "text/plain", "Peripherals not ready\n");
      return;
    }
    if (!Config.save(*_ctx)) {
      sServer.send(500, "text/plain", "Brightness applied but save failed\n");
      return;
    }
    sServer.send(200, "text/plain", "Brightness " + String(_ctx->config.ringBrightness) + "% saved\n");
    return;
  }

  if (action == "buzzer_volume") {
    if (!_ctx || !sServer.hasArg("value")) {
      sServer.send(400, "text/plain", "Missing volume value\n");
      return;
    }
    const uint8_t volume = static_cast<uint8_t>(sServer.arg("value").toInt());
    if (!LedBuzzer.setBuzzerVolume(volume, _ctx->config)) {
      sServer.send(503, "text/plain", "Peripherals not ready\n");
      return;
    }
    if (!Config.save(*_ctx)) {
      sServer.send(500, "text/plain", "Volume applied but save failed\n");
      return;
    }
    sServer.send(200, "text/plain", "Buzzer volume " + String(_ctx->config.buzzerVolume) + "% saved\n");
    return;
  }

  bool ok = false;

  if (action == "ring_demo") {
    if (!sServer.hasArg("value")) {
      sServer.send(400, "text/plain", "Missing pattern id\n");
      return;
    }
    const uint8_t patternId = static_cast<uint8_t>(sServer.arg("value").toInt());
    const uint8_t r = sServer.hasArg("r") ? static_cast<uint8_t>(sServer.arg("r").toInt()) : 0;
    const uint8_t g = sServer.hasArg("g") ? static_cast<uint8_t>(sServer.arg("g").toInt()) : 120;
    const uint8_t b = sServer.hasArg("b") ? static_cast<uint8_t>(sServer.arg("b").toInt()) : 255;
    ok = LedBuzzer.playRingDemo(patternId, r, g, b);
  } else if (action == "ring_stop") {
    ok = LedBuzzer.stopRingDemo();
  } else if (action == "buzzer_demo") {
    if (!sServer.hasArg("value")) {
      sServer.send(400, "text/plain", "Missing demo id\n");
      return;
    }
    ok = LedBuzzer.playBuzzerDemo(static_cast<uint8_t>(sServer.arg("value").toInt()));
    if (ok) {
      sServer.send(200, "application/json", "{\"success\":true,\"melody_playing\":true}");
      return;
    }
    sServer.send(503, "application/json", "{\"success\":false,\"melody_playing\":false}");
    return;
  } else if (action == "buzzer_melody_stop") {
    ok = LedBuzzer.stopBuzzerMelody();
    sServer.send(200, "application/json", ok ? "{\"success\":true,\"melody_playing\":false}"
                                              : "{\"success\":false}");
    return;
  } else if (action == "wifi_preview") {
    const uint8_t r = sServer.hasArg("r") ? static_cast<uint8_t>(sServer.arg("r").toInt()) : 0;
    const uint8_t g = sServer.hasArg("g") ? static_cast<uint8_t>(sServer.arg("g").toInt()) : 180;
    const uint8_t b = sServer.hasArg("b") ? static_cast<uint8_t>(sServer.arg("b").toInt()) : 60;
    const uint8_t ledPat = sServer.hasArg("led_pattern") ? static_cast<uint8_t>(sServer.arg("led_pattern").toInt()) : 0;
    const uint8_t ringPat = sServer.hasArg("ring_pattern") ? static_cast<uint8_t>(sServer.arg("ring_pattern").toInt()) : 0;
    const uint8_t rr = sServer.hasArg("ring_r") ? static_cast<uint8_t>(sServer.arg("ring_r").toInt()) : 0;
    const uint8_t rg = sServer.hasArg("ring_g") ? static_cast<uint8_t>(sServer.arg("ring_g").toInt()) : 40;
    const uint8_t rb = sServer.hasArg("ring_b") ? static_cast<uint8_t>(sServer.arg("ring_b").toInt()) : 255;
    WifiLed.setTheme(r, g, b, ledPat);
    WifiLed.setState(WifiLedState::ApActive);
    RgbRing.setWifiScanStyle(ringPat, rr, rg, rb);
    RgbRing.applyWifiScanLook();
    ok = true;
  } else if (action == "ring" || action == "buzzer" || action == "buzzer_tune") {
    if (!sServer.hasArg("value")) {
      sServer.send(400, "text/plain", "Missing value\n");
      return;
    }
    const String value = sServer.arg("value");
    if (action == "ring") {
      ok = LedBuzzer.playRing(value.c_str());
    } else if (action == "buzzer") {
      ok = LedBuzzer.playBuzzer(value.c_str());
    } else if (action == "buzzer_tune") {
      ok = LedBuzzer.playBuzzerTune(static_cast<uint8_t>(value.toInt()));
    }
  } else {
    ok = false;
  }

  if (action == "wifi_preview" || action == "ring_demo" || action == "ring_stop" || action == "ring" ||
      action == "buzzer" || action == "buzzer_tune") {
    if (!ok) {
      sServer.send(503, "text/plain", "Peripherals not ready or unknown action\n");
      return;
    }
    sServer.send(200, "text/plain", "OK\n");
    return;
  }
#endif

  sServer.send(400, "text/plain", "Unknown action\n");
}

void WebStatusServer::handleSetupApi() {
  if (!_ctx) {
    sServer.send(500, "text/plain", "Context not ready\n");
    return;
  }

  if (sServer.hasArg("name")) {
    strlcpy(_ctx->config.ownerName, sServer.arg("name").c_str(), sizeof(_ctx->config.ownerName));
  }
  if (sServer.hasArg("title")) {
    strlcpy(_ctx->config.title, sServer.arg("title").c_str(), sizeof(_ctx->config.title));
  }
  if (sServer.hasArg("email")) {
    strlcpy(_ctx->config.email, sServer.arg("email").c_str(), sizeof(_ctx->config.email));
  }

  if (sServer.hasArg("lab_pin")) {
    String labPin = sServer.arg("lab_pin");
    labPin.trim();
    if (labPin.length() > 0) {
      if (labPin.length() < 4 || labPin.length() > 15) {
        sServer.send(400, "text/plain", "Lab PIN must be 4–15 characters\n");
        return;
      }
      strlcpy(_ctx->config.labModePin, labPin.c_str(), sizeof(_ctx->config.labModePin));
    }
  }

#if APP_WIFI_PORTAL
  if (sServer.hasArg("captive")) {
    _ctx->config.captivePortalEnabled = sServer.arg("captive") == "1";
    WifiCaptive.setEnabled(_ctx->config.captivePortalEnabled);
  }
#endif

  if (sServer.hasArg("ap_ssid")) {
    const String ssid = sServer.arg("ap_ssid");
    if (ssid.length() == 0 || ssid.length() > 31) {
      sServer.send(400, "text/plain", "SSID must be 1–31 characters\n");
      return;
    }
    String password = sServer.hasArg("ap_password") ? sServer.arg("ap_password") : _ctx->config.apPassword;
    if (password.length() > 0 && password.length() < 8) {
      sServer.send(400, "text/plain", "Wi-Fi password must be at least 8 characters\n");
      return;
    }
#if APP_WIFI_AP_CONTROL
    const WifiApApplyResult apResult =
        WifiApControl.apply(*_ctx, ssid.c_str(), password.c_str(), _ctx->config.apChannel);
    if (!apResult.success) {
      sServer.send(400, "text/plain", apResult.error[0] ? apResult.error : "Could not apply AP settings\n");
      return;
    }
#else
    strlcpy(_ctx->config.apSsid, ssid.c_str(), sizeof(_ctx->config.apSsid));
    strlcpy(_ctx->config.apPassword, password.c_str(), sizeof(_ctx->config.apPassword));
    if (!WifiAp.begin(_ctx->config.apSsid, _ctx->config.apPassword, _ctx->config.apChannel)) {
      sServer.send(500, "text/plain", "AP restart failed\n");
      return;
    }
#endif
  }

  _ctx->config.firstBootComplete = true;

  if (!Config.save(*_ctx)) {
    sServer.send(500, "text/plain", "Could not save config to flash\n");
    return;
  }

  sServer.send(200, "text/plain", "Setup saved! Redirecting to dashboard...\n");
}

void WebStatusServer::handleDiagApi() {
#if APP_SYSTEM_DIAG
  const DiagReport report = Diagnostics.run(hackcardPeripheralsReady());
  sServer.send(200, "application/json", Diagnostics.toJson(report));
#else
  sServer.send(503, "application/json", "{\"all_pass\":false,\"items\":[]}");
#endif
}

void WebStatusServer::handleSerialLogApi() {
  uint32_t cursor = 0;
  if (sServer.hasArg("since")) {
    cursor = static_cast<uint32_t>(sServer.arg("since").toInt());
  }
  uint32_t next = cursor;
  const String chunk = SerialLog.readSince(next);
  sServer.sendHeader("X-Log-Cursor", String(next));
  sServer.send(200, "text/plain", chunk);
}

void WebStatusServer::handleWifiMonitorApsApi() {
#if APP_WIFI_MONITOR
  sServer.send(200, "application/json", WifiMonitor.apsToJson());
#else
  sServer.send(503, "application/json", "{\"count\":0,\"aps\":[]}");
#endif
}

void WebStatusServer::handleWifiMonitorLiveApi() {
#if APP_WIFI_MONITOR
  sServer.send(200, "application/json", WifiMonitor.liveToJson());
#else
  sServer.send(503, "application/json", "{\"count\":0,\"live\":[]}");
#endif
}

void WebStatusServer::handleWifiStaGetApi() {
  if (!_ctx) {
    sServer.send(503, "application/json", "{\"success\":false,\"error\":\"Not ready\"}");
    return;
  }
  char pinError[64] = {0};
  const String pin = sServer.arg("pin");
  if (!labPinCheck(pin.c_str(), pinError, sizeof(pinError))) {
    String json = "{\"success\":false,\"error\":\"";
    json += pinError;
    json += "\"}";
    sServer.send(403, "application/json", json);
    return;
  }
  if (!_ctx->config.staCredentialsSaved || _ctx->config.staSsid[0] == '\0') {
    sServer.send(200, "application/json", "{\"success\":true,\"saved\":false}");
    return;
  }
  String json = "{\"success\":true,\"saved\":true,\"ssid\":\"";
  json += _ctx->config.staSsid;
  json += "\",\"password\":\"";
  json += _ctx->config.staPassword;
  json += "\"}";
  sServer.send(200, "application/json", json);
}

void WebStatusServer::handleWifiReconStatusApi() {
#if APP_WIFI_RECON
  sServer.send(200, "application/json", WifiRecon.statusToJson());
#else
  sServer.send(503, "application/json", "{\"sta_held\":false}");
#endif
}

void WebStatusServer::handleWifiReconConnectApi() {
#if APP_WIFI_RECON
  const String pin = sServer.arg("pin");
  const String ssid = sServer.arg("ssid");
  const String password = sServer.hasArg("password") ? sServer.arg("password") : "";
  const WifiReconResult result = WifiRecon.connectHold(pin.c_str(), ssid.c_str(), password.c_str());
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiRecon.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiReconReleaseApi() {
#if APP_WIFI_RECON
  const WifiReconResult result = WifiRecon.release();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"status\":";
  json += WifiRecon.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiReconSubnetApi() {
#if APP_WIFI_RECON
  const WifiReconResult result = WifiRecon.subnetScan();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"hosts\":";
  json += WifiRecon.hostsToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiReconHostsApi() {
#if APP_WIFI_RECON
  sServer.send(200, "application/json", WifiRecon.hostsToJson());
#else
  sServer.send(503, "application/json", "{\"count\":0,\"hosts\":[]}");
#endif
}

void WebStatusServer::handleWifiReconPortScanApi() {
#if APP_WIFI_RECON
  const String ip = sServer.arg("ip");
  const String ports = sServer.hasArg("ports") ? sServer.arg("ports") : "top20";
  const uint16_t timeout = sServer.hasArg("timeout") ? static_cast<uint16_t>(sServer.arg("timeout").toInt()) : 120;
  const WifiReconResult result = WifiRecon.portScan(ip.c_str(), ports.c_str(), timeout);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"result\":";
  json += WifiRecon.portScanToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiReconPortsApi() {
#if APP_WIFI_RECON
  sServer.send(200, "application/json", WifiRecon.portScanToJson());
#else
  sServer.send(503, "application/json", "{\"open_ports\":[]}");
#endif
}

void WebStatusServer::handleWifiKarmaStatusApi() {
#if APP_WIFI_KARMA
  sServer.send(200, "application/json", WifiKarma.statusToJson());
#else
  sServer.send(503, "application/json", "{\"running\":false}");
#endif
}

void WebStatusServer::handleWifiKarmaStartApi() {
#if APP_WIFI_KARMA
  const String pin = sServer.arg("pin");
  const String ssids = sServer.hasArg("ssids") ? sServer.arg("ssids") : "Free_WiFi";
  const WifiKarmaResult result = WifiKarma.start(pin.c_str(), ssids.c_str());
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiKarma.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiKarmaStopApi() {
#if APP_WIFI_KARMA
  const WifiKarmaResult result = WifiKarma.stop();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"status\":";
  json += WifiKarma.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiCaptureStatusApi() {
#if APP_WIFI_CAPTURE
  sServer.send(200, "application/json", WifiCapture.statusToJson());
#else
  sServer.send(503, "application/json", "{\"running\":false}");
#endif
}

void WebStatusServer::handleWifiCaptureStartApi() {
#if APP_WIFI_CAPTURE
  const String pin = sServer.arg("pin");
  const uint8_t channel = sServer.hasArg("channel") ? static_cast<uint8_t>(sServer.arg("channel").toInt()) : 0;
  const uint8_t minutes = sServer.hasArg("minutes") ? static_cast<uint8_t>(sServer.arg("minutes").toInt()) : 5;
  const bool saveSd = sServer.hasArg("save_sd") && sServer.arg("save_sd") != "0" && sServer.arg("save_sd") != "false";
  const WifiCaptureResult result = WifiCapture.start(pin.c_str(), channel, minutes, saveSd);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"status\":";
  json += WifiCapture.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiCaptureStopApi() {
#if APP_WIFI_CAPTURE
  const WifiCaptureResult result = WifiCapture.stop();
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"status\":";
  json += WifiCapture.statusToJson();
  json += "}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiCaptureDownloadApi() {
#if APP_WIFI_CAPTURE
  if (!WifiCapture.hasCapture()) {
    sServer.send(404, "text/plain", "No capture file\n");
    return;
  }
  const size_t total = WifiCapture.pcapSize();
  if (total == 0 || total > 512000) {
    sServer.send(413, "text/plain", "Capture too large for download\n");
    return;
  }
  uint8_t* buf = static_cast<uint8_t*>(malloc(total));
  if (!buf) {
    sServer.send(500, "text/plain", "Out of memory\n");
    return;
  }
  size_t readLen = 0;
  if (!WifiCapture.readPcapBytes(buf, total, readLen)) {
    free(buf);
    sServer.send(500, "text/plain", "Read failed\n");
    return;
  }
  sServer.sendHeader("Content-Disposition", "attachment; filename=hackcard_capture.pcap");
  sServer.send(200, "application/vnd.tcpdump.pcap", "");
  WiFiClient client = sServer.client();
  client.write(buf, readLen);
  free(buf);
#else
  sServer.send(503, "text/plain", "Capture disabled\n");
#endif
}

void WebStatusServer::handleWifiCaptureVerifyApi() {
#if APP_WIFI_CAPTURE
  const String password = sServer.arg("password");
  bool matches = false;
  const WifiCaptureResult result = WifiCapture.verifyPassword(password.c_str(), matches);
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"matches\":";
  json += matches ? "true" : "false";
  json += ",\"message\":\"";
  json += result.error;
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false}");
#endif
}

void WebStatusServer::handleWifiTrainingExportApi() {
#if APP_WIFI_TRAINING
  const String log = WifiTraining.getAllCapturesText();
  sServer.sendHeader("Content-Disposition", "attachment; filename=training_captures.log");
  sServer.send(200, "text/plain", log.isEmpty() ? "(no captures yet)\n" : log);
#else
  sServer.send(503, "text/plain", "Training disabled\n");
#endif
}

void WebStatusServer::handleWifiCaptureExportSdApi() {
#if APP_WIFI_CAPTURE
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const bool ok = WifiCapture.exportCaptureToSd(path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"Capture disabled\"}");
#endif
}

void WebStatusServer::handleWifiTrainingExportSdApi() {
#if APP_WIFI_TRAINING
  if (respondSdExportUnavailable()) {
    return;
  }
  char path[72] = {0};
  char error[96] = {0};
  const bool ok = WifiTraining.exportCapturesToSd(path, sizeof(path), error, sizeof(error));
  String json = "{\"success\":";
  json += ok ? "true" : "false";
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += ",\"path\":\"";
  if (ok) {
    json += path;
  }
  json += "\",\"error\":\"";
  if (!ok) {
    json += error;
  }
  json += "\"}";
  sServer.send(200, "application/json", json);
#else
  sServer.send(503, "application/json", "{\"success\":false,\"sd_ready\":false,\"error\":\"Training disabled\"}");
#endif
}

void WebStatusServer::handleCliBanner() {
  sServer.send(200, "text/plain", Cli.getBanner());
}

void WebStatusServer::handleCliApi() {
  if (!sServer.hasArg("cmd")) {
    sServer.send(400, "text/plain", "Missing cmd parameter\n");
    return;
  }

  const String cmd = sServer.arg("cmd");
  sServer.send(200, "text/plain", Cli.executeCommand(cmd));
}


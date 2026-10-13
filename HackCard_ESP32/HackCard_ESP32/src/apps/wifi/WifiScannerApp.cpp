#include "WifiScannerApp.h"

#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/hal/WiFiHardwareTest.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "../../platform/telemetry/StatusLed.h"

namespace {

TaskHandle_t gScanWorkerTask = nullptr;

}  // namespace

namespace {

constexpr size_t kScanJsonCapacity = 8192;

#if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 7
using ScanJsonDocument = JsonDocument;
#else
using ScanJsonDocument = StaticJsonDocument<kScanJsonCapacity>;
#endif

}  // namespace

WifiScannerApp WifiScanner;

namespace {

void scanFeedbackStart() {
  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
}

void scanFeedbackSuccess() {
  Status.setApActive(true);
  Status.setIdle();
  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
}

void scanFeedbackError() {
  Status.setError();
  if (Buzzer.isEnabled()) {
    Buzzer.playError();
  }
}

}  // namespace

bool WifiScannerApp::restoreAp() {
  WifiHwTest.disconnectSta();
  delay(100);
  if (!WifiAp.restart()) {
    return false;
  }
  Status.setApActive(true);
  return true;
}

bool WifiScannerApp::ensureApUpAfterScan() {
  if (WifiAp.verifyRunning()) {
    Status.setApActive(true);
    return true;
  }
  if (!WifiAp.restart()) {
    return false;
  }
  Status.setApActive(true);
  return true;
}

bool WifiScannerApp::saveScanFailure(const char* message) {
  WifiScannerResult result;
  result.success = false;
  if (message) {
    strncpy(result.error, message, sizeof(result.error) - 1);
  }
  return saveScan(result);
}

bool WifiScannerApp::saveScan(const WifiScannerResult& result) {
  Storage.ensureDirectory("/wifi");

  const String json = toJson(result);
  if (json.length() < 16) {
    return false;
  }
  _memoryCacheJson = json;
  _lastStorageOk = Storage.writeTextFile("/wifi/last_scan.json", json);
  return true;
}

bool WifiScannerApp::commitScanResult(WifiScannerResult& result) {
  result.savedPath = "/wifi/last_scan.json";
  if (!saveScan(result)) {
    strncpy(result.error, "Could not build scan results", sizeof(result.error) - 1);
    result.success = false;
    return false;
  }
  return true;
}

void WifiScannerApp::scanWorkerEntry(void* param) {
  WifiScannerApp* self = static_cast<WifiScannerApp*>(param);
  if (self) {
    self->runScanWorker();
  }
  gScanWorkerTask = nullptr;
  vTaskDelete(nullptr);
}

void WifiScannerApp::runScanWorker() {
  WifiScannerResult result;
  _keepApUp = WifiAp.verifyRunning();

  bool scanOk = false;
  if (_keepApUp) {
    scanOk = WifiHwTest.scanNetworksWithApUp(result.scan, result.error, sizeof(result.error));
  } else {
    scanOk = WifiHwTest.scanNetworksDetailed(result.scan, nullptr, /*verbose=*/false);
  }

  _lastScanFinishedMs = millis();

  if (!scanOk) {
    if (result.error[0] == '\0') {
      strncpy(result.error, "Scan failed", sizeof(result.error) - 1);
    }
    saveScanFailure(result.error);
    result.apRestored = _keepApUp ? ensureApUpAfterScan() : restoreAp();
    scanFeedbackError();
    if (result.apRestored) {
      Status.setIdle();
    }
    Log.warn("wifi", result.error);
    _scanActive = false;
    return;
  }

  result.success = true;
  if (!commitScanResult(result)) {
    saveScanFailure(result.error);
    scanFeedbackError();
    _scanActive = false;
    return;
  }

  result.apRestored = _keepApUp ? ensureApUpAfterScan() : restoreAp();
  if (!result.apRestored) {
    strncpy(result.error, "Scan saved but AP restart failed", sizeof(result.error) - 1);
    saveScanFailure(result.error);
    scanFeedbackError();
    _scanActive = false;
    return;
  }

  char summary[24];
  snprintf(summary, sizeof(summary), "%u networks", result.scan.count);
  Log.activity("wifi", "scan", summary);
  scanFeedbackSuccess();
  _scanActive = false;
}

bool WifiScannerApp::isScanning() const {
  return _scanActive || gScanWorkerTask != nullptr;
}

bool WifiScannerApp::requestAsyncScan() {
  if (_scanActive || gScanWorkerTask != nullptr) {
    return false;
  }
  if (WifiHwTest.isScanRunning()) {
    WiFi.scanDelete();
  }

  scanFeedbackStart();
  _scanActive = true;
  _scanStartMs = millis();

  if (xTaskCreatePinnedToCore(scanWorkerEntry, "hcWifiScan", 12288, this, 1, &gScanWorkerTask, 0) != pdPASS) {
    _scanActive = false;
    scanFeedbackError();
    saveScanFailure("Could not start scan worker");
    return false;
  }

  return true;
}

void WifiScannerApp::loop() {
  // Scans from the lab UI run in hcWifiScan (runScanWorker), not WiFi.scanComplete().
  if (gScanWorkerTask != nullptr) {
    return;
  }
  if (!_scanActive) {
    return;
  }

  const int scanStatus = WiFi.scanComplete();
  if (scanStatus == WIFI_SCAN_RUNNING) {
    if (millis() - _scanStartMs > 20000UL) {
      _scanActive = false;
      _lastScanFinishedMs = millis();
      WiFi.scanDelete();
      scanFeedbackError();
      if (!_keepApUp) {
        restoreAp();
      } else {
        ensureApUpAfterScan();
        Status.setIdle();
      }
      saveScanFailure("Scan timed out");
      Log.warn("wifi", "scan timeout");
    }
    return;
  }

  finishAsyncScan(scanStatus);
}

void WifiScannerApp::finishAsyncScan(int scanStatus) {
  _scanActive = false;
  _lastScanFinishedMs = millis();
  WifiScannerResult result;

  if (scanStatus == WIFI_SCAN_FAILED ||
      !WifiHwTest.collectAsyncScanResults(result.scan, result.error, sizeof(result.error))) {
    if (result.error[0] == '\0') {
      strncpy(result.error, "Scan failed", sizeof(result.error) - 1);
    }
    saveScanFailure(result.error);
    result.apRestored = _keepApUp ? ensureApUpAfterScan() : restoreAp();
    scanFeedbackError();
    if (result.apRestored) {
      Status.setIdle();
    }
    Log.warn("wifi", result.error);
    return;
  }

  result.success = true;
  result.savedPath = "/wifi/last_scan.json";
  if (!saveScan(result)) {
    strncpy(result.error, "Scan OK but could not save results", sizeof(result.error) - 1);
    result.success = false;
    saveScanFailure(result.error);
    result.apRestored = _keepApUp ? ensureApUpAfterScan() : restoreAp();
    scanFeedbackError();
    return;
  }

  result.apRestored = _keepApUp ? ensureApUpAfterScan() : restoreAp();

  if (!result.apRestored) {
    strncpy(result.error, "Scan saved but AP restart failed — run wifi restart", sizeof(result.error) - 1);
    scanFeedbackError();
    return;
  }

  char summary[24];
  snprintf(summary, sizeof(summary), "%u networks", result.scan.count);
  Log.activity("wifi", "scan", summary);
  scanFeedbackSuccess();
}

String WifiScannerApp::scanningStatusJson() const {
  return "{\"ready\":false,\"scanning\":true,\"success\":false,\"count\":0,\"networks\":[],\"error\":\"\"}";
}

WifiScannerResult WifiScannerApp::scanOnce() {
  WifiScannerResult result;

  scanFeedbackStart();

  if (!WifiHwTest.scanNetworksDetailed(result.scan, nullptr, /*verbose=*/false)) {
    strncpy(result.error, result.scan.error[0] ? result.scan.error : "Scan failed", sizeof(result.error) - 1);
    result.apRestored = restoreAp();
    if (!result.apRestored) {
      strncat(result.error, " | AP restart failed — run wifi restart", sizeof(result.error) - strlen(result.error) - 1);
    }
    scanFeedbackError();
    if (result.apRestored) {
      Status.setApActive(true);
      Status.setIdle();
    }
    return result;
  }

  result.success = true;
  result.savedPath = "/wifi/last_scan.json";
  if (!saveScan(result)) {
    strncpy(result.error, "Scan OK but could not save results", sizeof(result.error) - 1);
    result.success = false;
    result.apRestored = restoreAp();
    if (result.apRestored) {
      Status.setApActive(true);
      Status.setIdle();
    }
    scanFeedbackError();
    return result;
  }

  result.apRestored = restoreAp();
  if (!result.apRestored) {
    strncpy(result.error, "Scan saved but AP restart failed — run wifi restart", sizeof(result.error) - 1);
    scanFeedbackError();
    return result;
  }

  char summary[24];
  snprintf(summary, sizeof(summary), "%u networks", result.scan.count);
  Log.activity("wifi", "scan", summary);

  scanFeedbackSuccess();
  return result;
}

String WifiScannerApp::toJson(const WifiScannerResult& result) const {
  ScanJsonDocument doc;
  doc["ready"] = true;
  doc["success"] = result.success;
  doc["count"] = result.scan.count;
  doc["scan_ms"] = result.scan.scanDurationMs;
  doc["scan_finished_ms"] = _lastScanFinishedMs;
  doc["ap_restored"] = result.apRestored;
  doc["saved_path"] = result.savedPath;
  doc["error"] = result.error;
  doc["scanning"] = false;

  JsonArray networks = doc["networks"].to<JsonArray>();
  for (uint8_t i = 0; i < result.scan.count; i++) {
    const WifiScanNetwork& net = result.scan.networks[i];
    JsonObject item = networks.add<JsonObject>();
    item["ssid"] = net.ssid;
    item["rssi"] = net.rssi;
    item["channel"] = net.channel;
    item["auth"] = net.auth;
    item["bssid"] = net.bssid;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

String WifiScannerApp::toText(const WifiScannerResult& result) const {
  if (!result.success) {
    String text = "── Wi-Fi Scan FAILED ──\n";
    text += result.error;
    text += "\n";
    if (result.apRestored) {
      text += "AP restored: yes\n";
    }
    return text;
  }

  String text = "── Wi-Fi Scan OK ──\n";
  text += "Networks: " + String(result.scan.count) + "\n";
  text += "Duration: " + String(result.scan.scanDurationMs) + " ms\n";
  text += "Saved:    " + result.savedPath + "\n\n";

  for (uint8_t i = 0; i < result.scan.count; i++) {
    const WifiScanNetwork& net = result.scan.networks[i];
    text += String(i + 1) + ". ";
    text += net.ssid;
    text += "  ";
    text += String(net.rssi);
    text += " dBm  ch:";
    text += String(net.channel);
    text += "  ";
    text += net.auth;
    text += "\n";
  }
  return text;
}

String WifiScannerApp::readLastScanJson() const {
  String raw;
  if (!Storage.readTextFile("/wifi/last_scan.json", raw)) {
    return "{\"ready\":false,\"success\":false,\"count\":0,\"networks\":[],\"error\":\"No saved scan\"}";
  }
  return raw;
}

String WifiScannerApp::lastScanApiJson() const {
  ScanJsonDocument doc;

  const String* source = nullptr;
  String fileRaw;
  if (_memoryCacheJson.length() >= 16) {
    source = &_memoryCacheJson;
  } else if (hasLastScan()) {
    fileRaw = readLastScanJson();
    if (fileRaw.length() >= 16) {
      source = &fileRaw;
    }
  }

  if (source != nullptr) {
    if (deserializeJson(doc, *source) != DeserializationError::Ok) {
      doc.clear();
      doc["ready"] = false;
      doc["success"] = false;
      doc["count"] = 0;
      doc["error"] = "Saved scan data is invalid";
      doc["networks"].to<JsonArray>();
    }
  } else {
    doc["ready"] = false;
    doc["success"] = false;
    doc["count"] = 0;
    doc["networks"].to<JsonArray>();
    doc["error"] = _scanActive ? "Scan in progress" : "No saved scan yet";
  }

  doc["scanning"] = _scanActive || gScanWorkerTask != nullptr;
  doc["storage_flash_ready"] = Storage.isFlashReady();
  doc["storage_saved"] = _lastStorageOk;
  doc["storage_tier"] = Storage.getTierLabel();
  if (_scanActive) {
    doc["scan_started_ms"] = _scanStartMs;
  }
  if (_lastScanFinishedMs > 0) {
    doc["scan_finished_ms"] = _lastScanFinishedMs;
  }

  String output;
  serializeJson(doc, output);
  return output;
}

bool WifiScannerApp::hasLastScan() const {
  return Storage.exists("/wifi/last_scan.json");
}

bool WifiScannerApp::clearLastScan() {
  if (_scanActive || gScanWorkerTask != nullptr) {
    return false;
  }
  static const char kCleared[] =
      "{\"ready\":false,\"success\":false,\"count\":0,\"networks\":[],\"cleared\":true}";
  _memoryCacheJson = kCleared;
  _lastStorageOk = false;
  _lastScanFinishedMs = 0;
  Storage.ensureDirectory("/wifi");
  if (Storage.writeTextFile("/wifi/last_scan.json", kCleared)) {
    _lastStorageOk = true;
  }
  return true;
}

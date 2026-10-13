#include "BleLabApp.h"

#include "config/user_config.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBle.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/telemetry/StatusLed.h"
#include "BleContactCardApp.h"

#include <BLEDevice.h>
#include <BLEUtils.h>

BleLabApp BleLab;

namespace {

constexpr char kLastScanPath[] = "/ble/last_scan.json";

}  // namespace

bool BleLabApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

bool BleLabApp::saveScan(const BleScanResult& result) {
  Storage.ensureDirectory("/ble");
  return Storage.writeTextFile(kLastScanPath, scanToJson(result));
}

BleScanResult BleLabApp::scanOnce(uint8_t seconds) {
  BleScanResult result;

  if (BleContactCard.isActive()) {
    strncpy(result.error, "Stop BLE contact card first", sizeof(result.error) - 1);
    return result;
  }

  if (seconds == 0) {
    seconds = 5;
  }
  if (seconds > 15) {
    seconds = 15;
  }

  Status.setScanning();
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }

  if (!Ble.ensureInit()) {
    strncpy(result.error, "BLE init failed", sizeof(result.error) - 1);
    Status.setError();
    if (Buzzer.isEnabled()) {
      Buzzer.playError();
    }
    return result;
  }

  const uint32_t startMs = millis();
  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(1349);
  scan->setWindow(449);

  BLEScanResults* found = scan->start(seconds, false);
  result.scanDurationMs = millis() - startMs;

  if (!found) {
    strncpy(result.error, "BLE scan failed", sizeof(result.error) - 1);
    Status.setError();
    if (Buzzer.isEnabled()) {
      Buzzer.playError();
    }
    return result;
  }

  for (int i = 0; i < found->getCount() && result.count < 20; i++) {
    BLEAdvertisedDevice device = found->getDevice(i);
    BleScanDevice& entry = result.devices[result.count];
    if (device.haveName()) {
      strlcpy(entry.name, device.getName().c_str(), sizeof(entry.name));
    } else {
      strlcpy(entry.name, "(no name)", sizeof(entry.name));
    }
    strlcpy(entry.address, device.getAddress().toString().c_str(), sizeof(entry.address));
    entry.rssi = device.getRSSI();
    result.count++;
  }

  scan->clearResults();
  result.success = true;
  saveScan(result);

  Status.setIdle();
  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("ble", "scan", String(result.count).c_str());

  return result;
}

BleAdvertiseResult BleLabApp::startAdvertise(const char* labPin, const char* name) {
  BleAdvertiseResult result;

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (BleContactCard.isActive()) {
    strncpy(result.error, "Stop BLE contact card first", sizeof(result.error) - 1);
    return result;
  }
  if (!name || name[0] == '\0') {
    strncpy(result.error, "Advertise name required", sizeof(result.error) - 1);
    return result;
  }
  if (strlen(name) > 29) {
    strncpy(result.error, "Name too long (max 29 chars)", sizeof(result.error) - 1);
    return result;
  }

  if (!Ble.startAdvertising(name)) {
    strncpy(result.error, "Could not start BLE advertising", sizeof(result.error) - 1);
    return result;
  }

  _advertising = true;
  strlcpy(_advertName, name, sizeof(_advertName));

  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
  Log.activity("ble", "advertise_start", _advertName);

  result.success = true;
  result.advertising = true;
  return result;
}

BleAdvertiseResult BleLabApp::stopAdvertise() {
  BleAdvertiseResult result;

  Ble.stopAdvertising();
  _advertising = false;
  _advertName[0] = '\0';

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("ble", "advertise_stop", "ok");

  result.success = true;
  result.advertising = false;
  return result;
}

String BleLabApp::scanToJson(const BleScanResult& result) const {
  String json = "{\"success\":";
  json += result.success ? "true" : "false";
  json += ",\"count\":";
  json += String(result.count);
  json += ",\"scan_ms\":";
  json += String(result.scanDurationMs);
  json += ",\"error\":\"";
  json += result.error;
  json += "\",\"devices\":[";
  for (uint8_t i = 0; i < result.count; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"name\":\"";
    json += result.devices[i].name;
    json += "\",\"address\":\"";
    json += result.devices[i].address;
    json += "\",\"rssi\":";
    json += String(result.devices[i].rssi);
    json += "}";
  }
  json += "]}";
  return json;
}

String BleLabApp::readLastScanJson() const {
  String json;
  if (Storage.readTextFile(kLastScanPath, json) && json.length() > 0) {
    return json;
  }
  return "{\"success\":false,\"count\":0,\"devices\":[],\"error\":\"No saved scan\"}";
}

bool BleLabApp::hasLastScan() const {
  return Storage.exists(kLastScanPath);
}

String BleLabApp::statusToJson() const {
  String json = "{\"ble_ready\":";
  json += Ble.isReady() ? "true" : "false";
  json += ",\"advertising\":";
  json += _advertising ? "true" : "false";
  json += ",\"advert_name\":\"";
  json += _advertName;
  json += "\",\"has_last_scan\":";
  json += hasLastScan() ? "true" : "false";
  json += "}";
  return json;
}

#include "BleContactCardApp.h"

#include "../demo/LedBuzzerApp.h"
#include "../../core/Logger.h"
#include "../../platform/hal/HalBle.h"
#include "../../platform/hal/HalBuzzer.h"
#include "BleLabApp.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

BleContactCardApp BleContactCard;

namespace {

constexpr char kServiceUuid[] = "0000fc00-0000-1000-8000-00805f9b34fb";
constexpr char kVcardCharUuid[] = "0000fc01-0000-1000-8000-00805f9b34fb";
constexpr char kNameCharUuid[] = "0000fc02-0000-1000-8000-00805f9b34fb";
constexpr char kEmailCharUuid[] = "0000fc03-0000-1000-8000-00805f9b34fb";

BLEServer* sServer = nullptr;
BLEService* sService = nullptr;
BLECharacteristic* sCharVcard = nullptr;
BLECharacteristic* sCharName = nullptr;
BLECharacteristic* sCharEmail = nullptr;

}  // namespace

bool BleContactCardApp::setupService(const HackCardConfig& config) {
  if (!Ble.ensureInit()) {
    return false;
  }

  if (!sServer) {
    sServer = BLEDevice::createServer();
  }
  if (!sServer) {
    return false;
  }

  if (!sService) {
    sService = sServer->createService(kServiceUuid);
    sCharVcard = sService->createCharacteristic(kVcardCharUuid, BLECharacteristic::PROPERTY_READ);
    sCharName = sService->createCharacteristic(kNameCharUuid, BLECharacteristic::PROPERTY_READ);
    sCharEmail = sService->createCharacteristic(kEmailCharUuid, BLECharacteristic::PROPERTY_READ);
    sService->start();
    _serviceReady = true;
  }

  const String vcard = buildVcard(config);
  sCharVcard->setValue(vcard.c_str());
  sCharName->setValue(config.ownerName[0] ? config.ownerName : "HackCard");
  sCharEmail->setValue(config.email[0] ? config.email : "you@example.com");
  return true;
}

bool BleContactCardApp::startAdvertising(const HackCardConfig& config) {
  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->reset();
  adv->addServiceUUID(kServiceUuid);

  BLEAdvertisementData adData;
  adData.setFlags(0x06);
  adData.setName(config.ownerName[0] ? config.ownerName : "HackCard");
  adv->setAdvertisementData(adData);
  adv->setScanResponseData(adData);
  adv->setMinPreferred(0x06);
  adv->start();
  return true;
}

void BleContactCardApp::stopAdvertising() {
  if (!Ble.isReady()) {
    return;
  }
  BLEDevice::getAdvertising()->stop();
}

BleContactCardResult BleContactCardApp::start(const HackCardConfig& config) {
  BleContactCardResult result;

  if (_active) {
    strncpy(result.error, "Contact card already active", sizeof(result.error) - 1);
    return result;
  }
  if (BleLab.isAdvertising()) {
    BleLab.stopAdvertise();
  }
  Ble.stopAdvertising();

  if (!setupService(config)) {
    strncpy(result.error, "BLE contact service setup failed", sizeof(result.error) - 1);
    return result;
  }
  if (!startAdvertising(config)) {
    strncpy(result.error, "BLE advertising failed", sizeof(result.error) - 1);
    return result;
  }

  _active = true;
  strlcpy(_contactName, config.ownerName[0] ? config.ownerName : "HackCard", sizeof(_contactName));

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("ble", "contact_card_start", _contactName);

  result.success = true;
  result.active = true;
  return result;
}

BleContactCardResult BleContactCardApp::stop() {
  BleContactCardResult result;

  stopAdvertising();
  _active = false;
  _contactName[0] = '\0';

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("ble", "contact_card_stop", "ok");

  result.success = true;
  result.active = false;
  return result;
}

BleContactCardResult BleContactCardApp::refresh(const HackCardConfig& config) {
  BleContactCardResult result;

  if (!_active) {
    strncpy(result.error, "Contact card not active", sizeof(result.error) - 1);
    return result;
  }

  if (!setupService(config)) {
    strncpy(result.error, "Could not refresh contact data", sizeof(result.error) - 1);
    return result;
  }

  result.success = true;
  result.active = true;
  return result;
}

String BleContactCardApp::statusToJson() const {
  String json = "\"contact_card\":";
  json += _active ? "true" : "false";
  json += ",\"contact_name\":\"";
  json += _contactName;
  json += "\"";
  return json;
}

#include "HalBle.h"

#include <BLEDevice.h>
#include <BLEUtils.h>

HalBle Ble;

bool HalBle::ensureInit() {
  if (_ready) {
    return true;
  }

  BLEDevice::init("HackCard");
  _ready = true;
  Serial.println("[ble] Stack initialized");
  return true;
}

bool HalBle::startAdvertising(const char* name) {
  if (!name || name[0] == '\0') {
    return false;
  }

  ensureInit();
  stopAdvertising();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  BLEAdvertisementData adData;
  adData.setFlags(0x06);
  adData.setName(name);

  adv->setAdvertisementData(adData);
  adv->setScanResponseData(adData);
  adv->setMinPreferred(0x06);
  adv->setMinInterval(0x20);
  adv->setMaxInterval(0x40);
  adv->start();

  _advertising = true;
  Serial.printf("[ble] Advertising as \"%s\"\n", name);
  return true;
}

void HalBle::stopAdvertising() {
  if (!_ready || !_advertising) {
    return;
  }

  BLEDevice::getAdvertising()->stop();
  _advertising = false;
  Serial.println("[ble] Advertising stopped");
}

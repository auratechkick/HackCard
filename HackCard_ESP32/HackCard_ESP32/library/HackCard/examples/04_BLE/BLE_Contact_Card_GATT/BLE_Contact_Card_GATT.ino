/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: BLE contact card GATT
 * FULL FIRMWARE: /ble Contact card
 *
 * WHAT IT DOES:
 * Exposes name/URL over GATT like the dashboard contact card.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Pair/read from a phone BLE app.
 *
 * LIBRARIES: HackCard, BLE
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

static const char* kName = "Alex Rivera";
static const char* kEmail = "alex@example.com";
static const char* kTitle = "Security Researcher";
static const char* kOrg = "HackCard Lab";
static const char* kUrl = "https://auratechkick.github.io/HackCard/";

static const char* kServiceUuid = "0000fc00-0000-1000-8000-00805f9b34fb";
static const char* kVcardUuid = "0000fc01-0000-1000-8000-00805f9b34fb";
static const char* kNameUuid = "0000fc02-0000-1000-8000-00805f9b34fb";
static const char* kEmailUuid = "0000fc03-0000-1000-8000-00805f9b34fb";

String buildVcard() {
  String v = "BEGIN:VCARD\nVERSION:3.0\nFN:";
  v += kName; v += "\nTITLE:"; v += kTitle; v += "\nORG:";
  v += kOrg; v += "\nEMAIL:"; v += kEmail; v += "\nURL:";
  v += kUrl; v += "\nEND:VCARD\n";
  return v;
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — BLE Contact Card GATT (web: /ble contact)");

  RgbRing.begin(50);
  Buzzer.begin(true);
  Ble.ensureInit();

  BLEServer* server = BLEDevice::createServer();
  BLEService* service = server->createService(kServiceUuid);
  BLECharacteristic* vcard = service->createCharacteristic(kVcardUuid, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic* name = service->createCharacteristic(kNameUuid, BLECharacteristic::PROPERTY_READ);
  BLECharacteristic* email = service->createCharacteristic(kEmailUuid, BLECharacteristic::PROPERTY_READ);

  String vc = buildVcard();
  vcard->setValue(vc.c_str());
  name->setValue(kName);
  email->setValue(kEmail);
  service->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(kServiceUuid);
  BLEAdvertisementData ad;
  ad.setFlags(0x06);
  ad.setName(kName);
  adv->setAdvertisementData(ad);
  adv->setScanResponseData(ad);
  adv->start();

  Serial.printf("Advertising as \"%s\" with contact GATT service.\n", kName);
  Serial.println("Use nRF Connect → connect → read FC01/FC02/FC03.");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(20);
}

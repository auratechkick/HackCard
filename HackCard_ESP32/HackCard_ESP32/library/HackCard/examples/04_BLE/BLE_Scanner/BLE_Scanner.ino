/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: BLE scanner
 * FULL FIRMWARE: /ble Scan
 *
 * WHAT IT DOES:
 * Scans nearby BLE devices and prints address/RSSI/name.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Open Serial; scan results stream periodically.
 *
 * LIBRARIES: HackCard, BLE
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

class PrintCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice device) override {
    Serial.printf("  %s  %s  RSSI=%d\n",
                  device.getAddress().toString().c_str(),
                  device.haveName() ? device.getName().c_str() : "(no name)",
                  device.getRSSI());
  }
};

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — BLE Scanner");

  RgbRing.begin(40);
  Ble.ensureInit();
  BLEDevice::getScan()->setAdvertisedDeviceCallbacks(new PrintCallbacks());
  BLEDevice::getScan()->setActiveScan(true);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println();
  Serial.println("Scanning 5s...");
  BLEScanResults* results = BLEDevice::getScan()->start(5, false);
  int count = results ? results->getCount() : 0;
  Serial.printf("Seen %d advertisements (callback printed live).\n", count);
  BLEDevice::getScan()->clearResults();
  RgbRing.setPattern(RingPattern::Success);
  delay(2000);
}

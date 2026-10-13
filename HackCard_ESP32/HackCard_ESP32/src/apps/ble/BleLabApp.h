#pragma once

#include <Arduino.h>

struct BleScanDevice {
  char name[32] = {0};
  char address[18] = {0};
  int rssi = 0;
};

struct BleScanResult {
  bool success = false;
  uint8_t count = 0;
  BleScanDevice devices[20] = {};
  uint32_t scanDurationMs = 0;
  char error[64] = {0};
};

struct BleAdvertiseResult {
  bool success = false;
  bool advertising = false;
  char error[64] = {0};
};

class BleLabApp {
public:
  BleScanResult scanOnce(uint8_t seconds = 5);
  BleAdvertiseResult startAdvertise(const char* labPin, const char* name);
  BleAdvertiseResult stopAdvertise();

  bool isAdvertising() const { return _advertising; }
  const char* getAdvertName() const { return _advertName; }

  String scanToJson(const BleScanResult& result) const;
  String readLastScanJson() const;
  bool hasLastScan() const;
  String statusToJson() const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool saveScan(const BleScanResult& result);

  bool _advertising = false;
  char _advertName[32] = {0};
};

extern BleLabApp BleLab;

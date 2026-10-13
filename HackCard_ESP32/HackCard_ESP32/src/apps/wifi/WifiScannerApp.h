#pragma once

#include "../../platform/hal/WiFiHardwareTest.h"

struct WifiScannerResult {
  bool success = false;
  WifiScanResult scan;
  bool apRestored = false;
  String savedPath;
  char error[64] = {0};
};

class WifiScannerApp {
public:
  WifiScannerResult scanOnce();
  bool requestAsyncScan();
  bool isScanning() const;
  void loop();
  String toJson(const WifiScannerResult& result) const;
  String toText(const WifiScannerResult& result) const;
  String readLastScanJson() const;
  String lastScanApiJson() const;
  String scanningStatusJson() const;
  bool hasLastScan() const;
  /** Clears in-memory and saved scan (no-op while scan running). */
  bool clearLastScan();

private:
  static void scanWorkerEntry(void* param);
  void runScanWorker();
  void finishAsyncScan(int scanStatus);
  bool commitScanResult(WifiScannerResult& result);
  bool saveScan(const WifiScannerResult& result);
  bool saveScanFailure(const char* message);
  bool restoreAp();
  bool ensureApUpAfterScan();

  bool _scanActive = false;
  bool _keepApUp = false;
  bool _lastStorageOk = false;
  uint32_t _scanStartMs = 0;
  uint32_t _lastScanFinishedMs = 0;
  String _memoryCacheJson;
};

extern WifiScannerApp WifiScanner;

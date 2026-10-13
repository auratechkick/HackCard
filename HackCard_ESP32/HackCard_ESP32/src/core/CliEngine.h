#pragma once

#include "AppContext.h"

#include <Arduino.h>

class CliEngine {
public:
  void begin(AppContext* ctx, bool serialEnabled = false);

  // Web terminal API
  String getBanner() const;
  String executeCommand(const String& line);

  // Optional USB serial CLI
  void feed(char c);

private:
  void write(const char* text);
  void writeLine(const char* text);
  void writeChar(char c);
  void printf(const char* fmt, ...);
  void executeLineInternal(const String& line);
  void printBanner();
  void printHelp();
  void printStatus();
  void printVersion();
  void printMemory();
  void printSdStatus();
  void printWifiStatus();
  void printWifiDiag();
  void printWifiRestart();
  void printWifiTest();
  void printWifiScan();
  void printWifiConnect(const String& args);
  void printWifiPortalStatus();
  void printWifiPortalSet(bool enabled);
  void printWifiApShow();
  void printWifiApSet(const String& args);
  void printWifiBeaconStatus();
  void printWifiBeaconStart(const String& args);
  void printWifiBeaconStop();
  void printWifiTwinStatus();
  void printWifiTwinStart(const String& args);
  void printWifiTwinStop();
  void printWifiTrainingStatus();
  void printWifiTrainingStart(const String& args);
  void printWifiTrainingStop();
  void printWifiTrainingCaptures();
  void printWifiTrainingCapturesClear();
  void printWifiMonitorStatus();
  void printWifiMonitorSniffStart(const String& args);
  void printWifiMonitorSniffStop();
  void printWifiMonitorDeauthStart(const String& args);
  void printWifiMonitorDeauthStop();
  void printWifiMonitorProbesClear();
  void printWifiMonitorLive();
  void printWifiReconStatus();
  void printWifiReconConnect(const String& args);
  void printWifiReconRelease();
  void printWifiReconSubnet();
  void printWifiKarmaStatus();
  void printWifiKarmaStart(const String& args);
  void printWifiKarmaStop();
  void printWifiCaptureStatus();
  void printWifiCaptureStart(const String& args);
  void printWifiCaptureStop();
  void printBleStatus();
  void printBleScan();
  void printBleAdvertiseStart(const String& args);
  void printBleAdvertiseStop();
  void printBleContactStart();
  void printBleContactStop();
  void printBleContactRefresh();
  void printHidStatus();
  void printHidOsSet(const String& name);
  void printHidRun(const String& args);
  void printHidPayloads();
  void printHidScriptRun(const String& args);
  void printHidScriptArm(const String& args);
  void printHidScriptDisarm();
  void printDiagRun();
  void printProfileShow();
  void printSettingsShow();
  void printSettingsSet(const String& args);
  void printLedCommand(const String& pattern);
  void printBuzzerCommand(const String& tone);
  void printLogsShow();
  void printLogsClear();
  void printModeShow();
  void printModeSet(const String& name);
  void printNfcStatus();
  void printNfcRead();
  void printNfcInfo();
  void printNfcDump();
  void printNfcWrite(const String& url);
  void printNfcHistory();
  void printNfcErase(const String& pin);
  void printNfcClone(const String& args);
  void printNfcKill(const String& pin);
  void printNfcClassicDump(const String& args);
  void printNfcClassicClone(const String& args);
  void printNfcKeysScan(const String& args);
  void printConfig();
  void printUnknown(const String& cmd);
  void printPrompt();

  AppContext* _ctx = nullptr;
  String _line;
  String _capture;
  bool _capturing = false;
  bool _serialEnabled = false;
};

extern CliEngine Cli;

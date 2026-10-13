#include "CliEngine.h"

#include "config/board_config.h"
#include "config/app_registry.h"
#include "ConfigStore.h"
#include "Logger.h"
#include "SerialLogBuffer.h"
#include "StorageManager.h"

#include <cstdarg>
#include <cstring>
#include <esp_heap_caps.h>

#include "src/platform/hal/HalWifiAp.h"
#include "src/platform/hal/WiFiDiagnostics.h"
#include "src/platform/hal/WiFiHardwareTest.h"
#if APP_WIFI_PORTAL
#include "src/platform/hal/WifiCaptivePortal.h"
#endif
#if APP_WIFI_SCANNER
#include "src/apps/wifi/WifiScannerApp.h"
#include "src/apps/wifi/WifiConnectApp.h"
#endif
#if APP_WIFI_AP_CONTROL
#include "src/apps/wifi/WifiApControlApp.h"
#endif
#if APP_WIFI_BEACON
#include "src/apps/wifi/WifiBeaconApp.h"
#endif
#if APP_WIFI_EVIL_TWIN
#include "src/apps/wifi/WifiEvilTwinApp.h"
#endif
#if APP_WIFI_TRAINING
#include "src/apps/wifi/WifiTrainingPortalApp.h"
#endif
#if APP_WIFI_MONITOR
#include "src/apps/wifi/WifiMonitorApp.h"
#endif
#if APP_WIFI_RECON
#include "src/apps/wifi/WifiReconApp.h"
#endif
#if APP_WIFI_KARMA
#include "src/apps/wifi/WifiKarmaApp.h"
#endif
#if APP_WIFI_CAPTURE
#include "src/apps/wifi/WifiCaptureApp.h"
#endif
#if APP_BLE
#include "src/apps/ble/BleLabApp.h"
#include "src/apps/ble/BleContactCardApp.h"
#endif
#include "BootState.h"
#include "src/apps/nfc/NfcReadApp.h"
#include "src/apps/nfc/NfcInfoApp.h"
#include "src/apps/nfc/NfcDumpApp.h"
#include "src/apps/nfc/NfcWriteApp.h"
#include "src/apps/nfc/NfcHistory.h"
#include "src/apps/nfc/NfcLabApp.h"
#include "src/apps/nfc/NfcClassicApp.h"
#include "src/apps/demo/LedBuzzerApp.h"
#include "src/platform/hal/HalBuzzer.h"
#if APP_USB_HID
#include "src/apps/usb/HidLabApp.h"
#include "src/apps/usb/HidPayloadStore.h"
#endif
#include "src/apps/system/ActivityLog.h"
#include "src/apps/system/DiagnosticsApp.h"
#include "DeviceModeManager.h"

CliEngine Cli;

void CliEngine::begin(AppContext* ctx, bool serialEnabled) {
  _ctx = ctx;
  _serialEnabled = serialEnabled;

  if (_serialEnabled) {
    printBanner();
  }
}

void CliEngine::write(const char* text) {
  if (_capturing) {
    _capture += text;
  }
  if (text && text[0] != '\0') {
    SerialLog.append(text, strlen(text));
  }
  if (_serialEnabled) {
    Serial.print(text);
  }
}

void CliEngine::writeLine(const char* text) {
  write(text);
  write("\n");
}

void CliEngine::writeChar(char c) {
  if (_capturing) {
    _capture += c;
  }
  SerialLog.append(&c, 1);
  if (_serialEnabled) {
    Serial.print(c);
  }
}

void CliEngine::printf(const char* fmt, ...) {
  char buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  write(buffer);
}

void CliEngine::printPrompt() {
  write("hackcard> ");
}

String CliEngine::getBanner() const {
  String banner;
  banner += "\n";
  banner += "╔══════════════════════════════════════╗\n";
  banner += "║  HackCard Terminal  (Phase 16)       ║\n";
  banner += "║  ";
  banner += BOARD_NAME;
  banner += " ";
  banner += FIRMWARE_VERSION;
  banner += "\n";
  banner += "║  Type 'help' for commands            ║\n";
  banner += "╚══════════════════════════════════════╝\n";
  banner += "hackcard> ";
  return banner;
}

String CliEngine::executeCommand(const String& line) {
  _capture = "";
  _capturing = true;

  executeLineInternal(line);

  _capturing = false;
  _capture += "\nhackcard> ";
  return _capture;
}

void CliEngine::feed(char c) {
  if (!_serialEnabled) {
    return;
  }

  if (c == '\r') {
    return;
  }

  if (c == '\n') {
    writeLine("");
    executeLineInternal(_line);
    _line = "";
    printPrompt();
    return;
  }

  if (c == '\b' || c == 127) {
    if (!_line.isEmpty()) {
      _line.remove(_line.length() - 1);
      write("\b \b");
    }
    return;
  }

  if (_line.length() < 240) {
    _line += c;
    writeChar(c);
  }
}

void CliEngine::executeLineInternal(const String& line) {
  String trimmed = line;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    return;
  }

  if (trimmed == "help") {
    printHelp();
    return;
  }
  if (trimmed == "status") {
    printStatus();
    return;
  }
  if (trimmed == "version") {
    printVersion();
    return;
  }
  if (trimmed == "memory") {
    printMemory();
    return;
  }
  if (trimmed == "sd status") {
    printSdStatus();
    return;
  }
  if (trimmed == "wifi status") {
    printWifiStatus();
    return;
  }
  if (trimmed == "wifi diag") {
    printWifiDiag();
    return;
  }
  if (trimmed == "wifi restart") {
    printWifiRestart();
    return;
  }
  if (trimmed == "wifi test") {
    printWifiTest();
    return;
  }
  if (trimmed == "wifi scan") {
    printWifiScan();
    return;
  }
  if (trimmed.startsWith("wifi connect ")) {
    printWifiConnect(trimmed.substring(strlen("wifi connect ")));
    return;
  }
  if (trimmed == "wifi portal status" || trimmed == "wifi portal") {
    printWifiPortalStatus();
    return;
  }
  if (trimmed == "wifi portal on") {
    printWifiPortalSet(true);
    return;
  }
  if (trimmed == "wifi portal off") {
    printWifiPortalSet(false);
    return;
  }
  if (trimmed == "wifi ap" || trimmed == "wifi ap show") {
    printWifiApShow();
    return;
  }
  if (trimmed.startsWith("wifi ap set ")) {
    printWifiApSet(trimmed.substring(strlen("wifi ap set ")));
    return;
  }
  if (trimmed == "wifi beacon status" || trimmed == "wifi beacon") {
    printWifiBeaconStatus();
    return;
  }
  if (trimmed == "wifi beacon stop") {
    printWifiBeaconStop();
    return;
  }
  if (trimmed.startsWith("wifi beacon start ")) {
    printWifiBeaconStart(trimmed.substring(strlen("wifi beacon start ")));
    return;
  }
  if (trimmed == "wifi twin status" || trimmed == "wifi twin") {
    printWifiTwinStatus();
    return;
  }
  if (trimmed == "wifi twin stop") {
    printWifiTwinStop();
    return;
  }
  if (trimmed.startsWith("wifi twin start ")) {
    printWifiTwinStart(trimmed.substring(strlen("wifi twin start ")));
    return;
  }
  if (trimmed == "wifi training status" || trimmed == "wifi training") {
    printWifiTrainingStatus();
    return;
  }
  if (trimmed == "wifi training stop") {
    printWifiTrainingStop();
    return;
  }
  if (trimmed.startsWith("wifi training start ")) {
    printWifiTrainingStart(trimmed.substring(strlen("wifi training start ")));
    return;
  }
  if (trimmed == "wifi training captures") {
    printWifiTrainingCaptures();
    return;
  }
  if (trimmed == "wifi training captures clear") {
    printWifiTrainingCapturesClear();
    return;
  }
  if (trimmed == "wifi monitor status" || trimmed == "wifi monitor") {
    printWifiMonitorStatus();
    return;
  }
  if (trimmed == "wifi monitor sniff stop") {
    printWifiMonitorSniffStop();
    return;
  }
  if (trimmed.startsWith("wifi monitor sniff start ")) {
    printWifiMonitorSniffStart(trimmed.substring(strlen("wifi monitor sniff start ")));
    return;
  }
  if (trimmed == "wifi monitor deauth stop") {
    printWifiMonitorDeauthStop();
    return;
  }
  if (trimmed.startsWith("wifi monitor deauth start ")) {
    printWifiMonitorDeauthStart(trimmed.substring(strlen("wifi monitor deauth start ")));
    return;
  }
  if (trimmed == "wifi monitor probes clear") {
    printWifiMonitorProbesClear();
    return;
  }
  if (trimmed == "wifi monitor live") {
    printWifiMonitorLive();
    return;
  }
#if APP_WIFI_RECON
  if (trimmed == "wifi recon status" || trimmed == "wifi recon") {
    printWifiReconStatus();
    return;
  }
  if (trimmed == "wifi recon release") {
    printWifiReconRelease();
    return;
  }
  if (trimmed == "wifi recon subnet") {
    printWifiReconSubnet();
    return;
  }
  if (trimmed.startsWith("wifi recon connect ")) {
    printWifiReconConnect(trimmed.substring(strlen("wifi recon connect ")));
    return;
  }
#endif
#if APP_WIFI_KARMA
  if (trimmed == "wifi karma status" || trimmed == "wifi karma") {
    printWifiKarmaStatus();
    return;
  }
  if (trimmed == "wifi karma stop") {
    printWifiKarmaStop();
    return;
  }
  if (trimmed.startsWith("wifi karma start ")) {
    printWifiKarmaStart(trimmed.substring(strlen("wifi karma start ")));
    return;
  }
#endif
#if APP_WIFI_CAPTURE
  if (trimmed == "wifi capture status" || trimmed == "wifi capture") {
    printWifiCaptureStatus();
    return;
  }
  if (trimmed == "wifi capture stop") {
    printWifiCaptureStop();
    return;
  }
  if (trimmed.startsWith("wifi capture start ")) {
    printWifiCaptureStart(trimmed.substring(strlen("wifi capture start ")));
    return;
  }
#endif
  if (trimmed == "ble status" || trimmed == "ble") {
    printBleStatus();
    return;
  }
  if (trimmed == "ble scan") {
    printBleScan();
    return;
  }
  if (trimmed == "ble advertise stop") {
    printBleAdvertiseStop();
    return;
  }
  if (trimmed == "ble contact start") {
    printBleContactStart();
    return;
  }
  if (trimmed == "ble contact stop") {
    printBleContactStop();
    return;
  }
  if (trimmed == "ble contact refresh") {
    printBleContactRefresh();
    return;
  }
  if (trimmed.startsWith("ble advertise start ")) {
    printBleAdvertiseStart(trimmed.substring(strlen("ble advertise start ")));
    return;
  }
#if APP_USB_HID
  if (trimmed == "hid status" || trimmed == "hid") {
    printHidStatus();
    return;
  }
  if (trimmed.startsWith("hid os ")) {
    printHidOsSet(trimmed.substring(strlen("hid os ")));
    return;
  }
  if (trimmed.startsWith("hid run ")) {
    printHidRun(trimmed.substring(strlen("hid run ")));
    return;
  }
  if (trimmed.startsWith("hid notepad ")) {
    printHidRun("notepad " + trimmed.substring(strlen("hid notepad ")));
    return;
  }
  if (trimmed.startsWith("hid lock ")) {
    printHidRun("lock " + trimmed.substring(strlen("hid lock ")));
    return;
  }
  if (trimmed.startsWith("hid type ")) {
    printHidRun("type " + trimmed.substring(strlen("hid type ")));
    return;
  }
  if (trimmed.startsWith("hid url ")) {
    printHidRun("url " + trimmed.substring(strlen("hid url ")));
    return;
  }
  if (trimmed == "hid payloads") {
    printHidPayloads();
    return;
  }
  if (trimmed == "hid script disarm") {
    printHidScriptDisarm();
    return;
  }
  if (trimmed.startsWith("hid script run ")) {
    printHidScriptRun(trimmed.substring(strlen("hid script run ")));
    return;
  }
  if (trimmed.startsWith("hid script arm ")) {
    printHidScriptArm(trimmed.substring(strlen("hid script arm ")));
    return;
  }
#endif
  if (trimmed == "config show") {
    printConfig();
    return;
  }
  if (trimmed == "diag run") {
    printDiagRun();
    return;
  }
  if (trimmed == "profile show") {
    printProfileShow();
    return;
  }
  if (trimmed == "settings show") {
    printSettingsShow();
    return;
  }
  if (trimmed.startsWith("settings ")) {
    printSettingsSet(trimmed.substring(strlen("settings ")));
    return;
  }
  if (trimmed.startsWith("led ")) {
    printLedCommand(trimmed.substring(strlen("led ")));
    return;
  }
  if (trimmed.startsWith("buzzer ")) {
    printBuzzerCommand(trimmed.substring(strlen("buzzer ")));
    return;
  }
  if (trimmed == "logs show") {
    printLogsShow();
    return;
  }
  if (trimmed == "logs clear") {
    printLogsClear();
    return;
  }
  if (trimmed == "mode show") {
    printModeShow();
    return;
  }
  if (trimmed.startsWith("mode set ")) {
    printModeSet(trimmed.substring(strlen("mode set ")));
    return;
  }
  if (trimmed == "nfc status") {
    printNfcStatus();
    return;
  }
  if (trimmed == "nfc read") {
    printNfcRead();
    return;
  }
  if (trimmed == "nfc info") {
    printNfcInfo();
    return;
  }
  if (trimmed == "nfc dump") {
    printNfcDump();
    return;
  }
  if (trimmed.startsWith("nfc write ")) {
    printNfcWrite(trimmed.substring(strlen("nfc write ")));
    return;
  }
  if (trimmed == "nfc history") {
    printNfcHistory();
    return;
  }
  if (trimmed.startsWith("nfc erase ")) {
    printNfcErase(trimmed.substring(strlen("nfc erase ")));
    return;
  }
  if (trimmed.startsWith("nfc clone ")) {
    printNfcClone(trimmed.substring(strlen("nfc clone ")));
    return;
  }
  if (trimmed.startsWith("nfc kill ")) {
    printNfcKill(trimmed.substring(strlen("nfc kill ")));
    return;
  }
  if (trimmed.startsWith("nfc classic dump")) {
    printNfcClassicDump(trimmed.substring(strlen("nfc classic dump")));
    return;
  }
  if (trimmed.startsWith("nfc classic clone ")) {
    printNfcClassicClone(trimmed.substring(strlen("nfc classic clone ")));
    return;
  }
  if (trimmed.startsWith("nfc keys scan")) {
    printNfcKeysScan(trimmed.substring(strlen("nfc keys scan")));
    return;
  }
  if (trimmed == "clear") {
    return;
  }

  printUnknown(trimmed);
}

void CliEngine::printBanner() {
  write(getBanner().c_str());
}

void CliEngine::printHelp() {
  writeLine("Commands:");
  writeLine("  help          Show this list");
  writeLine("  status        Device status");
  writeLine("  version       Firmware and board info");
  writeLine("  memory        Heap and PSRAM usage");
  writeLine("  sd status     Storage tier and SD state");
  writeLine("  wifi status   Wi-Fi AP status");
  writeLine("  wifi diag     Detailed Wi-Fi hardware report");
  writeLine("  wifi restart  Restart the access point");
  writeLine("  wifi test     Full hardware test (scan + connect router)");
  writeLine("  wifi scan     Scan nearby networks");
  writeLine("  wifi connect <ssid> <password|open>  Connect to a network");
  writeLine("  wifi portal [on|off|status]  Captive portal landing page");
  writeLine("  wifi ap [show|set ssid X [pass Y] [ch N]]  AP settings");
  writeLine("  wifi beacon [status|start <pin>|stop]  Lab beacon demo");
  writeLine("  wifi twin [status|start <pin> <ssid>|stop]  Evil-twin sim");
  writeLine("  wifi training [status|start <pin>|stop|captures]  Awareness portal");
  writeLine("  wifi monitor [status|sniff|deauth|live|probes clear]  Probe/deauth lab");
  writeLine("  wifi recon [status|connect|release|subnet]  STA hold + subnet scan");
  writeLine("  wifi karma [status|start <pin>|stop]  Probe-response lure");
  writeLine("  wifi capture [status|start <pin>|stop]  EAPOL / handshake capture");
  writeLine("  ble [status|scan|advertise|contact]  BLE lab + contact card");
  writeLine("  hid [status|os|run|payloads|script]  USB HID + DuckyScript payloads");
  writeLine("  config show   Current profile/config source");
  writeLine("  profile show  Contact profile summary");
  writeLine("  settings show Device settings");
  writeLine("  settings brightness <10-100>");
  writeLine("  settings volume <10-100>");
  writeLine("  settings buzzer on|off");
  writeLine("  settings tune <success|error|boot|nfc> <0-7>");
  writeLine("  led idle|success|error|scan|boot|demo <1-12> [r g b]|stop");
  writeLine("  buzzer success|error|boot|nfc_*|tune <0-7>");
  writeLine("  logs show     Recent activity log entries");
  writeLine("  logs clear    Clear activity logs");
  writeLine("  mode show     Current device mode (business or lab)");
  writeLine("  mode set business|lab");
  writeLine("  nfc status    PN532 chip status");
  writeLine("  nfc read      Read NFC tag UID");
  writeLine("  nfc info      Read tag family + NDEF preview");
  writeLine("  nfc dump      Dump Type 2 tag pages to storage");
  writeLine("  nfc write <url>       Write NDEF URL to blank tag");
  writeLine("  nfc write text <msg>  Write NDEF plain text");
  writeLine("  nfc write vcard       Write profile as vCard contact");
  writeLine("  nfc history   Recent NFC activity");
  writeLine("  nfc erase <pin>  Erase Type 2 user pages (lab PIN)");
  writeLine("  nfc clone <pin> [dump_path]  Clone Type 2 dump to tag");
  writeLine("  nfc kill <pin>   Permanently lock Type 2 tag");
  writeLine("  nfc classic dump [key]  Dump Mifare Classic sectors");
  writeLine("  nfc classic clone <pin> [key]  Clone last classic dump");
  writeLine("  nfc keys scan [sector]  Try common Classic keys");
  writeLine("  diag run      Hardware self-test (RGB, buzzer, storage)");
  writeLine("  clear         Clear terminal screen");
}

void CliEngine::printStatus() {
  if (!_ctx) {
    return;
  }

  writeLine("── HackCard Status ──");
  printf("Owner:     %s\n", _ctx->config.ownerName);
  printf("AP SSID:   %s\n", _ctx->config.apSsid);
  printf("Storage:   %s\n", Storage.getTierLabel());
  printf("Config:    %s\n", Config.getSourceLabel());
  printf("Uptime:    %lu ms\n", millis());
}

void CliEngine::printVersion() {
  printf("Firmware:  %s\n", FIRMWARE_VERSION);
  printf("Board:     %s\n", BOARD_NAME);
  printf("MCU:       %s\n", BOARD_MCU);
  printf("Flash:     %d MB | PSRAM: %d MB\n", FLASH_SIZE_MB, PSRAM_SIZE_MB);
}

void CliEngine::printMemory() {
  printf("Free heap:  %u bytes\n", ESP.getFreeHeap());
  printf("Min heap:   %u bytes\n", ESP.getMinFreeHeap());
  printf("Free PSRAM: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void CliEngine::printSdStatus() {
  writeLine(Storage.getStatusLine());
  printf("Tier:       %s\n", Storage.getTierLabel());
  printf("SD ready:   %s\n", Storage.isSdReady() ? "yes" : "no");
  printf("Flash FS:   %s\n", Storage.isFlashReady() ? "yes" : "no");
}

void CliEngine::printWifiStatus() {
  writeLine("── Wi-Fi AP Status ──");
  writeLine(WifiAp.getStatusText().c_str());
  writeLine("Note: HackCard broadcasts on 2.4 GHz only.");
#if APP_WIFI_PORTAL
  if (_ctx) {
    printf("Captive portal: %s\n", _ctx->config.captivePortalEnabled ? "ON" : "OFF");
    printf("Landing page:   http://%s/portal\n", WifiAp.getIp().c_str());
  }
#endif
  printf("Open in browser: http://%s/terminal\n", WifiAp.getIp().c_str());
}

void CliEngine::printWifiDiag() {
  writeLine("── Wi-Fi Diagnostics ──");
  writeLine(WifiAp.getStatusText().c_str());
  if (WifiAp.getLastError().length() > 0) {
    writeLine(WifiAp.getLastError().c_str());
  }
  printf("AP IP: %s\n", WifiAp.getIp().c_str());
  printf("AP channel: %d\n", WifiAp.getChannel());
  writeLine("HackCard notes:");
  writeLine("- GPIO45 RGB ring is an ESP32-S3 strapping pin");
  writeLine("- GPIO37 buzzer PWM is detached while idle");
  writeLine("- SD SPI scan starts 8s after boot");
  writeLine("- AP must be 2.4 GHz on your phone");
}

void CliEngine::printWifiRestart() {
  if (WifiAp.restart()) {
#if APP_WIFI_PORTAL
    if (_ctx && _ctx->config.captivePortalEnabled) {
      WifiCaptive.setEnabled(true);
    }
#endif
    writeLine("Wi-Fi AP restarted successfully.");
    writeLine(WifiAp.getStatusText().c_str());
  } else {
    writeLine("Wi-Fi AP restart failed.");
    writeLine(WifiAp.getLastError().c_str());
  }
}

void CliEngine::printWifiTest() {
  writeLine("Running full Wi-Fi hardware test...");
  writeLine("(Step details print on Serial Monitor too)");
  const WiFiTestResult result =
      WifiHwTest.runFullTest(WIFI_TEST_STA_SSID, WIFI_TEST_STA_PASSWORD, WIFI_TEST_CONNECT_TIMEOUT);

  writeLine("── Test Summary ──");
  printf("Networks found : %d\n", result.networksFound);
  printf("Target seen    : %s\n", result.targetSeen ? "yes" : "no");
  printf("Connected      : %s\n", result.connectOk ? "yes" : "no");
  if (result.connectOk) {
    printf("IP             : %s\n", result.ip.c_str());
  }
  if (!result.failureReason.isEmpty()) {
    printf("Failure        : %s\n", result.failureReason.c_str());
  }
  writeLine("Restart AP with: wifi restart");
}

void CliEngine::printWifiScan() {
#if APP_WIFI_SCANNER
  writeLine("Scanning 2.4 GHz networks (~10s)...");
  writeLine("HackCard AP will restart after scan.");
  const WifiScannerResult result = WifiScanner.scanOnce();
  write(WifiScanner.toText(result).c_str());
#else
  writeLine("Scanning Wi-Fi networks...");
  const WiFiTestResult result = WifiHwTest.scanNetworks(WIFI_TEST_STA_SSID);
  printf("Found %d network(s)\n", result.networksFound);
  if (result.targetSeen) {
    printf("Target %s at %d dBm\n", WIFI_TEST_STA_SSID, result.targetRssi);
  }
  if (!result.failureReason.isEmpty()) {
    writeLine(result.failureReason.c_str());
  }
  writeLine("Restart AP with: wifi restart");
#endif
}

void CliEngine::printWifiConnect(const String& args) {
  String trimmed = args;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    writeLine("Usage: wifi connect <ssid> <password>");
    writeLine("       wifi connect <ssid> open          (open network)");
    writeLine("Example: wifi connect MyHomeRouter secret123");
    writeLine("Example: wifi connect CafeGuest open");
    return;
  }

  String ssid;
  String password;
  String auth;
  uint8_t channel = 0;

  int spaceIdx = trimmed.indexOf(' ');
  if (spaceIdx > 0) {
    ssid = trimmed.substring(0, spaceIdx);
    String rest = trimmed.substring(spaceIdx + 1);
    rest.trim();

    const int chIdx = rest.indexOf(" ch:");
    if (chIdx > 0) {
      channel = static_cast<uint8_t>(rest.substring(chIdx + 4).toInt());
      rest = rest.substring(0, chIdx);
      rest.trim();
    }

    if (rest.equalsIgnoreCase("open")) {
      auth = "OPEN";
    } else {
      password = rest;
    }
  } else {
    ssid = WIFI_TEST_STA_SSID;
    password = trimmed;
  }

  if (auth != "OPEN" && password.isEmpty()) {
    writeLine("Password missing. Use 'open' for open networks.");
    return;
  }

#if APP_WIFI_SCANNER
  writeLine("Connecting (HackCard AP restarts when done)...");
  const WifiConnectResult result =
      WifiConnect.connectOnce(ssid.c_str(), password.c_str(), channel, auth.c_str(), nullptr);
  write(WifiConnect.toText(result).c_str());
#else
  writeLine("Wi-Fi connect not enabled.");
#endif
}

void CliEngine::printWifiPortalStatus() {
#if APP_WIFI_PORTAL
  if (!_ctx) {
    return;
  }
  writeLine("── Captive Portal ──");
  printf("Enabled:  %s\n", _ctx->config.captivePortalEnabled ? "yes" : "no");
  printf("DNS:      %s\n", WifiCaptive.isRunning() ? "running" : "stopped");
  printf("Landing:  http://%s/portal\n", WifiAp.getIp().c_str());
  writeLine("When ON, phones joining the AP auto-open your profile page.");
#else
  writeLine("Captive portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiPortalSet(bool enabled) {
#if APP_WIFI_PORTAL
  if (!_ctx) {
    return;
  }
  _ctx->config.captivePortalEnabled = enabled;
  WifiCaptive.setEnabled(enabled);
  if (Config.save(*_ctx)) {
    printf("Captive portal %s\n", enabled ? "enabled" : "disabled");
  } else {
    writeLine("Could not save config.");
  }
#else
  (void)enabled;
  writeLine("Captive portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiApShow() {
#if APP_WIFI_AP_CONTROL
  if (!_ctx) {
    return;
  }
  const WifiApStatus st = WifiApControl.getStatus(_ctx->config);
  writeLine("── HackCard AP Settings ──");
  printf("Running:   %s\n", st.apRunning ? "ON" : "OFF");
  printf("SSID:      %s (config: %s)\n", st.runningSsid, st.configSsid);
  printf("IP:        %s\n", st.ip);
  printf("Channel:   %u (config: %u)\n", st.runningChannel, st.configChannel);
  printf("Portal:    %s\n", st.captivePortalEnabled ? "ON" : "OFF");
  writeLine("Change with: wifi ap set ssid MyCard pass secret123 ch 6");
#else
  writeLine("AP control not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiApSet(const String& args) {
#if APP_WIFI_AP_CONTROL
  if (!_ctx) {
    return;
  }

  String ssid = _ctx->config.apSsid;
  String password = _ctx->config.apPassword;
  uint8_t channel = _ctx->config.apChannel;

  String trimmed = args;
  trimmed.trim();
  int idx = 0;
  while (idx < trimmed.length()) {
    if (trimmed.startsWith("ssid ", idx)) {
      idx += 5;
      const int end = trimmed.indexOf(' ', idx);
      ssid = end > 0 ? trimmed.substring(idx, end) : trimmed.substring(idx);
      idx = end > 0 ? end : trimmed.length();
    } else if (trimmed.startsWith("pass ", idx)) {
      idx += 5;
      const int end = trimmed.indexOf(' ', idx);
      password = end > 0 ? trimmed.substring(idx, end) : trimmed.substring(idx);
      idx = end > 0 ? end : trimmed.length();
    } else if (trimmed.startsWith("ch ", idx)) {
      idx += 3;
      const int end = trimmed.indexOf(' ', idx);
      const String chStr = end > 0 ? trimmed.substring(idx, end) : trimmed.substring(idx);
      channel = static_cast<uint8_t>(chStr.toInt());
      idx = end > 0 ? end : trimmed.length();
    } else {
      writeLine("Usage: wifi ap set ssid <name> [pass <password>] [ch <1|6|11|0>]");
      return;
    }
    while (idx < trimmed.length() && trimmed.charAt(idx) == ' ') {
      idx++;
    }
  }

  writeLine("Applying AP settings...");
  const WifiApApplyResult result = WifiApControl.apply(*_ctx, ssid.c_str(), password.c_str(), channel);
  if (result.success) {
    writeLine("AP updated and restarted.");
    printf("Join SSID: %s\n", _ctx->config.apSsid);
  } else {
    writeLine(result.error);
  }
#else
  (void)args;
  writeLine("AP control not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiBeaconStatus() {
#if APP_WIFI_BEACON
  writeLine("── Wi-Fi Beacon Lab ──");
  write(WifiBeacon.statusToJson().c_str());
  writeLine("");
  writeLine(WifiBeacon.isRunning() ? "Status: RUNNING" : "Status: idle");
#else
  writeLine("Beacon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiBeaconStop() {
#if APP_WIFI_BEACON
  const WifiBeaconResult result = WifiBeacon.stop();
  writeLine(result.success ? "Beacon stopped." : "Beacon stop failed.");
#else
  writeLine("Beacon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiBeaconStart(const String& args) {
#if APP_WIFI_BEACON
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: wifi beacon start <pin> [SSID1 SSID2 ...]");
    writeLine("   or: wifi beacon start <pin>  (uses saved SSID list)");
    return;
  }

  const String pin = trimmed.substring(0, space);
  String ssidText = trimmed.substring(space + 1);
  ssidText.trim();
  if (ssidText.isEmpty()) {
    ssidText = WifiBeacon.readSavedSsids();
  } else {
    ssidText.replace(" ", "\n");
  }

  const WifiBeaconResult result = WifiBeacon.start(pin.c_str(), ssidText.c_str());
  if (result.success) {
    printf("Beacon started with %u SSID(s).\n", result.ssidCount);
  } else {
    writeLine(result.error);
  }
#else
  (void)args;
  writeLine("Beacon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTwinStatus() {
#if APP_WIFI_EVIL_TWIN
  writeLine("── Evil Twin Simulation ──");
  write(WifiEvilTwin.statusToJson().c_str());
  writeLine("");
  writeLine(WifiEvilTwin.isRunning() ? "Status: SIMULATION RUNNING" : "Status: idle");
#else
  writeLine("Evil-twin simulation not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTwinStop() {
#if APP_WIFI_EVIL_TWIN
  if (!_ctx) {
    writeLine("Context not ready");
    return;
  }
  const WifiEvilTwinResult result = WifiEvilTwin.stop(*_ctx);
  writeLine(result.success ? "Simulation stopped — AP restored." : "Stop failed.");
#else
  writeLine("Evil-twin simulation not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTwinStart(const String& args) {
#if APP_WIFI_EVIL_TWIN
  if (!_ctx) {
    writeLine("Context not ready");
    return;
  }

  String trimmed = args;
  trimmed.trim();
  const int pinEnd = trimmed.indexOf(' ');
  if (pinEnd <= 0) {
    writeLine("Usage: wifi twin start <pin> <ssid> [minutes] [channel]");
    return;
  }

  const String pin = trimmed.substring(0, pinEnd);
  trimmed = trimmed.substring(pinEnd + 1);
  trimmed.trim();

  const int ssidEnd = trimmed.indexOf(' ');
  String ssid;
  uint8_t minutes = 5;
  uint8_t channel = 0;

  if (ssidEnd > 0) {
    ssid = trimmed.substring(0, ssidEnd);
    trimmed = trimmed.substring(ssidEnd + 1);
    trimmed.trim();
    if (!trimmed.isEmpty()) {
      const int nextSpace = trimmed.indexOf(' ');
      const String minStr = nextSpace > 0 ? trimmed.substring(0, nextSpace) : trimmed;
      minutes = static_cast<uint8_t>(minStr.toInt());
      if (nextSpace > 0) {
        channel = static_cast<uint8_t>(trimmed.substring(nextSpace + 1).toInt());
      }
    }
  } else {
    ssid = trimmed;
  }

  const WifiEvilTwinResult result = WifiEvilTwin.start(*_ctx, pin.c_str(), ssid.c_str(), channel, minutes);
  if (result.success) {
    printf("Simulation started as \"%s\" for %u minute(s).\n", ssid.c_str(), minutes);
  } else {
    writeLine(result.error);
  }
#else
  (void)args;
  writeLine("Evil-twin simulation not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTrainingStatus() {
#if APP_WIFI_TRAINING
  writeLine("── Training Portal ──");
  write(WifiTraining.statusToJson().c_str());
  writeLine("");
  write(WifiTraining.configToJson().c_str());
  writeLine("");
  writeLine(WifiTraining.isActive() ? "Status: ACTIVE" : "Status: idle");
#else
  writeLine("Training portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTrainingStop() {
#if APP_WIFI_TRAINING
  const WifiTrainingResult result = WifiTraining.stop();
  writeLine(result.success ? "Training portal stopped." : "Stop failed.");
#else
  writeLine("Training portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTrainingStart(const String& args) {
#if APP_WIFI_TRAINING
  String pin = args;
  pin.trim();
  if (pin.isEmpty()) {
    writeLine("Usage: wifi training start <pin>");
    return;
  }
  const WifiTrainingResult result = WifiTraining.start(pin.c_str());
  writeLine(result.success ? "Training portal active — /portal shows login template." : result.error);
#else
  (void)args;
  writeLine("Training portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTrainingCaptures() {
#if APP_WIFI_TRAINING
  writeLine("── Training Captures ──");
  write(WifiTraining.getCapturesTail().c_str());
  writeLine("");
#else
  writeLine("Training portal not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiTrainingCapturesClear() {
#if APP_WIFI_TRAINING
  writeLine(WifiTraining.clearCaptures() ? "Capture log cleared." : "Could not clear log.");
#else
  writeLine("Training portal not enabled in app_registry.h");
#endif
}

void CliEngine::printBleContactStart() {
#if APP_BLE
  if (!_ctx) {
    writeLine("Context not ready");
    return;
  }
  const BleContactCardResult result = BleContactCard.start(_ctx->config);
  writeLine(result.success ? "BLE contact card started." : result.error);
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printBleContactStop() {
#if APP_BLE
  const BleContactCardResult result = BleContactCard.stop();
  writeLine(result.success ? "BLE contact card stopped." : "Stop failed.");
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printBleContactRefresh() {
#if APP_BLE
  if (!_ctx) {
    writeLine("Context not ready");
    return;
  }
  const BleContactCardResult result = BleContactCard.refresh(_ctx->config);
  writeLine(result.success ? "BLE contact card refreshed." : result.error);
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorStatus() {
#if APP_WIFI_MONITOR
  writeLine("── Wi-Fi Monitor Lab ──");
  write(WifiMonitor.statusToJson().c_str());
  writeLine("");
  write(WifiMonitor.probesToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorSniffStop() {
#if APP_WIFI_MONITOR
  const WifiMonitorResult result = WifiMonitor.stopSniff();
  writeLine(result.success ? "Probe sniff stopped." : "Stop failed.");
#else
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorSniffStart(const String& args) {
#if APP_WIFI_MONITOR
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: wifi monitor sniff start <pin> [minutes]");
    return;
  }
  const String pin = trimmed.substring(0, space);
  const uint8_t minutes = trimmed.substring(space + 1).toInt();
  const WifiMonitorResult result = WifiMonitor.startSniff(pin.c_str(), minutes);
  writeLine(result.success ? "Probe sniff started." : result.error);
#else
  (void)args;
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorDeauthStop() {
#if APP_WIFI_MONITOR
  const WifiMonitorResult result = WifiMonitor.stopDeauth();
  writeLine(result.success ? "Deauth demo stopped." : "Stop failed.");
#else
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorDeauthStart(const String& args) {
#if APP_WIFI_MONITOR
  String trimmed = args;
  trimmed.trim();
  const int p1 = trimmed.indexOf(' ');
  if (p1 <= 0) {
    writeLine("Usage: wifi monitor deauth start <pin> <bssid> [channel] [count] [mode]");
    return;
  }
  const String pin = trimmed.substring(0, p1);
  trimmed = trimmed.substring(p1 + 1);
  trimmed.trim();
  const int p2 = trimmed.indexOf(' ');
  const String bssid = p2 > 0 ? trimmed.substring(0, p2) : trimmed;
  uint8_t channel = 0;
  uint8_t count = 5;
  uint8_t attackMode = 0;
  if (p2 > 0) {
    trimmed = trimmed.substring(p2 + 1);
    trimmed.trim();
    const int p3 = trimmed.indexOf(' ');
    if (p3 > 0) {
      channel = static_cast<uint8_t>(trimmed.substring(0, p3).toInt());
      trimmed = trimmed.substring(p3 + 1);
      trimmed.trim();
      const int p4 = trimmed.indexOf(' ');
      if (p4 > 0) {
        count = static_cast<uint8_t>(trimmed.substring(0, p4).toInt());
        attackMode = static_cast<uint8_t>(trimmed.substring(p4 + 1).toInt());
      } else if (!trimmed.isEmpty()) {
        count = static_cast<uint8_t>(trimmed.toInt());
      }
    } else if (!trimmed.isEmpty()) {
      channel = static_cast<uint8_t>(trimmed.toInt());
    }
  }
  const WifiMonitorResult result = WifiMonitor.startDeauth(pin.c_str(), bssid.c_str(), channel, count, attackMode);
  writeLine(result.success ? "Deauth demo started." : result.error);
#else
  (void)args;
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorProbesClear() {
#if APP_WIFI_MONITOR
  writeLine(WifiMonitor.clearProbes() ? "Probe log cleared." : "Could not clear probe log.");
#else
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiMonitorLive() {
#if APP_WIFI_MONITOR
  write(WifiMonitor.liveToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi monitor not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiReconStatus() {
#if APP_WIFI_RECON
  write(WifiRecon.statusToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi recon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiReconConnect(const String& args) {
#if APP_WIFI_RECON
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: wifi recon connect <pin> <ssid> [password]");
    return;
  }
  const String pin = trimmed.substring(0, space);
  trimmed = trimmed.substring(space + 1);
  trimmed.trim();
  const int passSpace = trimmed.indexOf(' ');
  const String ssid = passSpace > 0 ? trimmed.substring(0, passSpace) : trimmed;
  const String password = passSpace > 0 ? trimmed.substring(passSpace + 1) : "";
  const WifiReconResult result = WifiRecon.connectHold(pin.c_str(), ssid.c_str(), password.c_str());
  writeLine(result.success ? "STA hold active." : result.error);
#else
  (void)args;
  writeLine("Wi-Fi recon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiReconRelease() {
#if APP_WIFI_RECON
  const WifiReconResult result = WifiRecon.release();
  writeLine(result.success ? "AP restored." : result.error);
#else
  writeLine("Wi-Fi recon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiReconSubnet() {
#if APP_WIFI_RECON
  const WifiReconResult result = WifiRecon.subnetScan();
  if (!result.success) {
    writeLine(result.error);
    return;
  }
  write(WifiRecon.hostsToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi recon not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiKarmaStatus() {
#if APP_WIFI_KARMA
  write(WifiKarma.statusToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi karma not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiKarmaStart(const String& args) {
#if APP_WIFI_KARMA
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: wifi karma start <pin> [ssid list comma-separated]");
    return;
  }
  const String pin = trimmed.substring(0, space);
  const String ssids = trimmed.substring(space + 1);
  const WifiKarmaResult result = WifiKarma.start(pin.c_str(), ssids.c_str());
  writeLine(result.success ? "Karma active." : result.error);
#else
  (void)args;
  writeLine("Wi-Fi karma not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiKarmaStop() {
#if APP_WIFI_KARMA
  const WifiKarmaResult result = WifiKarma.stop();
  writeLine(result.success ? "Karma stopped." : result.error);
#else
  writeLine("Wi-Fi karma not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiCaptureStatus() {
#if APP_WIFI_CAPTURE
  write(WifiCapture.statusToJson().c_str());
  writeLine("");
#else
  writeLine("Wi-Fi capture not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiCaptureStart(const String& args) {
#if APP_WIFI_CAPTURE
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: wifi capture start <pin> [channel] [minutes]");
    return;
  }
  const String pin = trimmed.substring(0, space);
  trimmed = trimmed.substring(space + 1);
  trimmed.trim();
  uint8_t channel = 0;
  uint8_t minutes = 5;
  const int chSpace = trimmed.indexOf(' ');
  if (chSpace > 0) {
    channel = static_cast<uint8_t>(trimmed.substring(0, chSpace).toInt());
    minutes = static_cast<uint8_t>(trimmed.substring(chSpace + 1).toInt());
  } else if (!trimmed.isEmpty()) {
    channel = static_cast<uint8_t>(trimmed.toInt());
  }
  const WifiCaptureResult result = WifiCapture.start(pin.c_str(), channel, minutes);
  writeLine(result.success ? "Capture started." : result.error);
#else
  (void)args;
  writeLine("Wi-Fi capture not enabled in app_registry.h");
#endif
}

void CliEngine::printWifiCaptureStop() {
#if APP_WIFI_CAPTURE
  const WifiCaptureResult result = WifiCapture.stop();
  writeLine(result.success ? "Capture stopped." : result.error);
#else
  writeLine("Wi-Fi capture not enabled in app_registry.h");
#endif
}

void CliEngine::printBleStatus() {
#if APP_BLE
  writeLine("── BLE Lab ──");
  write(BleLab.statusToJson().c_str());
  writeLine("");
  write("{");
  write(BleContactCard.statusToJson().c_str());
  writeLine("}");
  writeLine("");
  writeLine(BleContactCard.isActive() ? "Contact card: ACTIVE" : "Contact card: idle");
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printBleScan() {
#if APP_BLE
  writeLine("Scanning BLE for 5 seconds...");
  const BleScanResult result = BleLab.scanOnce(5);
  write(BleLab.scanToJson(result).c_str());
  writeLine("");
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printBleAdvertiseStop() {
#if APP_BLE
  const BleAdvertiseResult result = BleLab.stopAdvertise();
  writeLine(result.success ? "BLE advertising stopped." : "Stop failed.");
#else
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printBleAdvertiseStart(const String& args) {
#if APP_BLE
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: ble advertise start <pin> <name>");
    return;
  }
  const String pin = trimmed.substring(0, space);
  const String name = trimmed.substring(space + 1);
  const BleAdvertiseResult result = BleLab.startAdvertise(pin.c_str(), name.c_str());
  if (result.success) {
    printf("BLE advertising as \"%s\"\n", name.c_str());
  } else {
    writeLine(result.error);
  }
#else
  (void)args;
  writeLine("BLE not enabled in app_registry.h");
#endif
}

void CliEngine::printHidStatus() {
#if APP_USB_HID
  if (!_ctx) {
    return;
  }
  const HidLabResult result = HidLab.status();
  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  writeLine("── USB HID ──");
  printf("Ready:     %s\n", HidLab.isReady() ? "yes" : "no");
  printf("USB host:  %s\n", result.success ? "connected" : "not connected");
  printf("Target OS: %s\n", hidOsName(os));
  if (!result.success && result.error[0] != '\0') {
    printf("Note:      %s\n", result.error);
  }
  if (HidLab.lastMessage()[0] != '\0') {
    printf("Last:      %s\n", HidLab.lastMessage());
  }
  writeLine("Web:       /hid");
#else
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidOsSet(const String& name) {
#if APP_USB_HID
  if (!_ctx) {
    return;
  }
  String trimmed = name;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    writeLine("Usage: hid os windows|mac|linux");
    return;
  }
  const HidTargetOs os = hidOsFromString(trimmed.c_str());
  if (!HidLab.setTargetOs(_ctx->config, os)) {
    writeLine("Invalid OS");
    return;
  }
  if (!Config.save(*_ctx)) {
    writeLine("Failed to save config");
    return;
  }
  printf("HID target OS: %s\n", hidOsName(os));
#else
  (void)name;
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidPayloads() {
#if APP_USB_HID
  write(HidPayloads.listToJson().c_str());
  writeLine("");
#else
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidScriptDisarm() {
#if APP_USB_HID
  HidLab.disarmButtonPayload();
  writeLine("BOOT payload disarmed.");
#else
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidScriptRun(const String& args) {
#if APP_USB_HID
  if (!_ctx) {
    return;
  }
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: hid script run <pin> <filename.duck>");
    return;
  }
  const String pin = trimmed.substring(0, space);
  const String name = trimmed.substring(space + 1);
  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  const HidLabResult result = HidLab.runPayloadScript(pin.c_str(), name.c_str(), os);
  writeLine(result.success ? result.error : result.error);
#else
  (void)args;
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidScriptArm(const String& args) {
#if APP_USB_HID
  if (!_ctx) {
    return;
  }
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: hid script arm <pin> <filename.duck>");
    return;
  }
  const String pin = trimmed.substring(0, space);
  const String name = trimmed.substring(space + 1);
  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  const HidLabResult result = HidLab.armButtonPayload(pin.c_str(), name.c_str(), os);
  writeLine(result.error);
#else
  (void)args;
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printHidRun(const String& args) {
#if APP_USB_HID
  if (!_ctx) {
    return;
  }
  String trimmed = args;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    writeLine("Usage: hid run <action> <pin> [payload]");
    writeLine("Actions: notepad, calc, terminal, terminal_info, screenshot, lock,");
    writeLine("         alt_tab, mute, vol_up, vol_down, play, next, prev,");
    writeLine("         click [left|right|middle], move <dx,dy>, key <name>, type, url");
    return;
  }

  const int space = trimmed.indexOf(' ');
  if (space <= 0) {
    writeLine("Usage: hid run <action> <pin> [payload]");
    return;
  }

  String action = trimmed.substring(0, space);
  String rest = trimmed.substring(space + 1);
  rest.trim();
  action.trim();

  const int pinSpace = rest.indexOf(' ');
  String pin;
  String payload;
  if (pinSpace <= 0) {
    pin = rest;
  } else {
    pin = rest.substring(0, pinSpace);
    payload = rest.substring(pinSpace + 1);
    payload.trim();
  }
  pin.trim();

  if (pin.isEmpty()) {
    writeLine("Lab PIN required");
    return;
  }

  String apiAction = action;
  if (action == "lock") {
    apiAction = "lock_screen";
  } else if (action == "type") {
    apiAction = "type_text";
  } else if (action == "url") {
    apiAction = "open_website";
  } else if (action == "calc") {
    apiAction = "calculator";
  } else if (action == "alt_tab") {
    apiAction = "app_switcher";
  } else if (action == "close" || action == "alt_f4") {
    apiAction = "close_window";
  } else if (action == "mute") {
    apiAction = "volume_mute";
  } else if (action == "vol_up") {
    apiAction = "volume_up";
  } else if (action == "vol_down") {
    apiAction = "volume_down";
  } else if (action == "play") {
    apiAction = "media_play_pause";
  } else if (action == "next") {
    apiAction = "media_next";
  } else if (action == "prev") {
    apiAction = "media_previous";
  } else if (action == "click") {
    apiAction = "mouse_click";
    if (payload.isEmpty()) {
      payload = "left";
    }
  } else if (action == "move") {
    apiAction = "mouse_move";
  } else if (action == "key") {
    apiAction = "key_press";
  } else if (action == "sysinfo") {
    apiAction = "terminal_info";
  }

  const HidTargetOs os = HidLab.getTargetOs(_ctx->config);
  const HidLabResult result = HidLab.runAction(os, apiAction.c_str(), pin.c_str(), payload.c_str());
  writeLine(result.error);
#else
  (void)args;
  writeLine("USB HID not enabled in app_registry.h");
#endif
}

void CliEngine::printDiagRun() {
  const DiagReport report = Diagnostics.run(hackcardPeripheralsReady());
  write(Diagnostics.toText(report).c_str());
}

void CliEngine::printProfileShow() {
  if (!_ctx) {
    return;
  }

  writeLine("── Contact Profile ──");
  printf("Name:      %s\n", _ctx->config.ownerName);
  printf("Title:     %s\n", _ctx->config.title);
  printf("Company:   %s\n", _ctx->config.company);
  printf("Email:     %s\n", _ctx->config.email);
  printf("Phone:     %s\n", _ctx->config.phone);
  printf("Website:   %s\n", _ctx->config.website);
  printf("LinkedIn:  %s\n", _ctx->config.linkedin);
  printf("GitHub:    %s\n", _ctx->config.github);
  printf("Bio:       %s\n", _ctx->config.bio);
  writeLine("Edit at: http://192.168.4.1/profile/edit");
}

void CliEngine::printSettingsShow() {
  if (!_ctx) {
    return;
  }

  writeLine("── Settings ──");
  printf("Brightness:     %u\n", _ctx->config.ringBrightness);
  printf("Buzzer volume:  %u\n", _ctx->config.buzzerVolume);
  printf("Buzzer:         %s\n", _ctx->config.buzzerEnabled ? "on" : "off");
  printf("Tune success:   %s (%u)\n", HalBuzzer::tuneName(_ctx->config.tuneSuccess), _ctx->config.tuneSuccess);
  printf("Tune error:     %s (%u)\n", HalBuzzer::tuneName(_ctx->config.tuneError), _ctx->config.tuneError);
  printf("Tune boot:      %s (%u)\n", HalBuzzer::tuneName(_ctx->config.tuneBoot), _ctx->config.tuneBoot);
  printf("Tune nfc:       %s (%u)\n", HalBuzzer::tuneName(_ctx->config.tuneNfc), _ctx->config.tuneNfc);
  printf("Boot animation: %s\n", _ctx->config.bootAnimation ? "on" : "off");
  printf("Activity log:   %s\n", _ctx->config.activityLogging ? "on" : "off");
  printf("AP SSID:        %s\n", _ctx->config.apSsid);
  printf("Lab PIN:        (set — %u chars)\n", strlen(_ctx->config.labModePin));
#if APP_WIFI_PORTAL
  printf("Captive portal: %s\n", _ctx->config.captivePortalEnabled ? "on" : "off");
#endif
}

void CliEngine::printSettingsSet(const String& args) {
  if (!_ctx) {
    return;
  }

  String trimmed = args;
  trimmed.trim();
  if (trimmed.startsWith("brightness ")) {
    int value = trimmed.substring(strlen("brightness ")).toInt();
    if (value < 10) {
      value = 10;
    }
    if (value > 100) {
      value = 100;
    }
    _ctx->config.ringBrightness = static_cast<uint8_t>(value);
  } else if (trimmed.startsWith("volume ")) {
    int value = trimmed.substring(strlen("volume ")).toInt();
    if (value < 10) {
      value = 10;
    }
    if (value > 100) {
      value = 100;
    }
    _ctx->config.buzzerVolume = static_cast<uint8_t>(value);
  } else if (trimmed == "buzzer on") {
    _ctx->config.buzzerEnabled = true;
  } else if (trimmed == "buzzer off") {
    _ctx->config.buzzerEnabled = false;
  } else if (trimmed.startsWith("tune ")) {
    String rest = trimmed.substring(strlen("tune "));
    rest.trim();
    const int space = rest.indexOf(' ');
    if (space < 0) {
      writeLine("Usage: settings tune <success|error|boot|nfc> <0-7>");
      return;
    }
    const String event = rest.substring(0, space);
    const uint8_t tuneId = static_cast<uint8_t>(rest.substring(space + 1).toInt());
    if (tuneId >= static_cast<uint8_t>(BuzzerTuneId::Count)) {
      writeLine("Tune id must be 0-7.");
      return;
    }
    if (event == "success") {
      _ctx->config.tuneSuccess = tuneId;
    } else if (event == "error") {
      _ctx->config.tuneError = tuneId;
    } else if (event == "boot") {
      _ctx->config.tuneBoot = tuneId;
    } else if (event == "nfc") {
      _ctx->config.tuneNfc = tuneId;
    } else {
      writeLine("Usage: settings tune <success|error|boot|nfc> <0-7>");
      return;
    }
  } else {
    writeLine("Usage: settings brightness <10-100> | settings volume <10-100> | settings buzzer on|off | settings tune <event> <0-7>");
    return;
  }

  Config.save(*_ctx);
  LedBuzzer.applySettings(_ctx->config);
  writeLine("Settings saved.");
  printSettingsShow();
}

void CliEngine::printLedCommand(const String& pattern) {
  String trimmed = pattern;
  trimmed.trim();
  if (trimmed.startsWith("demo ")) {
    int patternId = 0;
    int r = 0;
    int g = 120;
    int b = 255;
    const int matched = sscanf(trimmed.c_str() + 5, "%d %d %d %d", &patternId, &r, &g, &b);
    if (matched < 1) {
      writeLine("Usage: led demo <1-12> [r g b]");
      return;
    }
    if (LedBuzzer.playRingDemo(static_cast<uint8_t>(patternId), static_cast<uint8_t>(r),
                               static_cast<uint8_t>(g), static_cast<uint8_t>(b))) {
      printf("Ring demo pattern %d (rgb %d,%d,%d)\n", patternId, r, g, b);
    } else {
      writeLine("Unknown demo pattern or peripherals not ready.");
      writeLine("Try: led demo <1-12> [r g b]");
    }
    return;
  }
  if (trimmed == "stop") {
    if (LedBuzzer.stopRingDemo()) {
      writeLine("Ring demo stopped.");
    } else {
      writeLine("Peripherals not ready.");
    }
    return;
  }
  if (LedBuzzer.playRing(trimmed.c_str())) {
    printf("Ring pattern: %s\n", trimmed.c_str());
  } else {
    writeLine("Unknown pattern or peripherals not ready.");
    writeLine("Try: idle, success, error, scan, boot, demo <1-12>, stop");
  }
}

void CliEngine::printBuzzerCommand(const String& tone) {
  String trimmed = tone;
  trimmed.trim();
  if (trimmed.startsWith("tune ")) {
    const uint8_t tuneId = static_cast<uint8_t>(trimmed.substring(strlen("tune ")).toInt());
    if (LedBuzzer.playBuzzerTune(tuneId)) {
      printf("Buzzer tune: %s (%u)\n", HalBuzzer::tuneName(tuneId), tuneId);
    } else {
      writeLine("Unknown tune or peripherals not ready.");
      writeLine("Try: buzzer tune <0-7>");
    }
    return;
  }
  if (LedBuzzer.playBuzzer(trimmed.c_str())) {
    printf("Buzzer: %s\n", trimmed.c_str());
  } else {
    writeLine("Unknown tone or peripherals not ready.");
    writeLine("Try: success, error, boot, nfc_ready, tune <0-7>");
  }
}

void CliEngine::printLogsShow() {
  String logs;
  ActivityLogs.readTail(logs, 30);
  writeLine("── Activity Logs (last 30) ──");
  write(logs.c_str());
}

void CliEngine::printLogsClear() {
  if (ActivityLogs.clear()) {
    Log.activity("logs", "clear", "ok");
    writeLine("Activity logs cleared.");
  } else {
    writeLine("Could not clear logs.");
  }
}

void CliEngine::printModeShow() {
  if (!_ctx) {
    return;
  }

  const DeviceMode mode = DeviceModeManager::fromStored(_ctx->config.deviceMode);
  const DeviceModeSpec ms = DeviceModeManager::spec(mode);
  writeLine("── Current Mode ──");
  printf("Mode:        %s (%s)\n", ms.title, DeviceModeManager::toString(mode));
  printf("Description: %s\n", ms.description);
}

void CliEngine::printModeSet(const String& name) {
  if (!_ctx) {
    return;
  }

  String trimmed = name;
  trimmed.trim();
  const DeviceMode mode = DeviceModeManager::fromString(trimmed.c_str());
  if (trimmed.isEmpty()) {
    writeLine("Usage: mode set business|lab");
    return;
  }

  DeviceModeManager::apply(*_ctx, mode);
  Config.save(*_ctx);
  printf("Mode set to: %s (%s)\n", DeviceModeManager::spec(mode).title, DeviceModeManager::toString(mode));
}

void CliEngine::printNfcStatus() {
  NfcRead.ensureReady();
  writeLine("── NFC Status ──");
  printf("Ready: %s\n", Nfc.isReady() ? "yes" : "no");
  printf("Chip:  %s\n", Nfc.getChipInfo());
  printf("I2C:   SDA=%d SCL=%d addr=0x24\n", PIN_NFC_SDA, PIN_NFC_SCL);
}

void CliEngine::printNfcRead() {
  writeLine("Hold tag near PN532...");
  const NfcReadResult result = NfcRead.readOnce();
  write(NfcRead.toText(result).c_str());
}

void CliEngine::printNfcInfo() {
  writeLine("Hold tag near PN532 (detailed scan)...");
  const NfcInfoResult result = NfcInfo.readInfo();
  write(NfcInfo.toText(result).c_str());
}

void CliEngine::printNfcDump() {
  writeLine("Hold tag near PN532 (page dump)...");
  const NfcDumpResult result = NfcDump.dumpOnce();
  write(NfcDump.toText(result).c_str());
  if (result.success && !result.preview.isEmpty()) {
    writeLine("");
    write(result.preview.c_str());
  }
}

void CliEngine::printNfcWrite(const String& args) {
  String trimmed = args;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    writeLine("Usage:");
    writeLine("  nfc write https://example.com");
    writeLine("  nfc write text Hello world");
    writeLine("  nfc write vcard");
    return;
  }

  if (trimmed.startsWith("text ")) {
    const String text = trimmed.substring(5);
    if (text.isEmpty()) {
      writeLine("Usage: nfc write text <message>");
      return;
    }
    writeLine("Hold blank NTAG/Ultralight tag near PN532...");
    const NfcWriteResult result = NfcWrite.writeText(text.c_str());
    if (result.success) {
      printf("NDEF text written: %s\n", result.summary);
    } else {
      printf("Write failed: %s\n", result.error);
    }
    return;
  }

  if (trimmed == "vcard") {
    if (!_ctx) {
      writeLine("Config not loaded");
      return;
    }
    writeLine("Hold blank NTAG/Ultralight tag near PN532...");
    const NfcWriteResult result = NfcWrite.writeVcard(_ctx->config);
    if (result.success) {
      printf("vCard written for: %s\n", result.summary);
    } else {
      printf("Write failed: %s\n", result.error);
    }
    return;
  }

  writeLine("Hold blank NTAG/Ultralight tag near PN532...");
  const NfcWriteResult result = NfcWrite.writeUri(trimmed.c_str());
  if (result.success) {
    printf("NDEF URL written: %s\n", result.summary);
  } else {
    printf("Write failed: %s\n", result.error);
  }
}

void CliEngine::printNfcHistory() {
  write(NfcHist.readRecent(20).c_str());
}

void CliEngine::printNfcErase(const String& pin) {
  String trimmed = pin;
  trimmed.trim();
  writeLine("Hold Type 2 tag near PN532 (erase user pages)...");
  const NfcEraseResult result = NfcWrite.eraseTag(trimmed.c_str());
  if (result.success) {
    writeLine("Tag user data erased.");
  } else {
    printf("Erase failed: %s\n", result.error);
  }
}

void CliEngine::printNfcClone(const String& args) {
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  String pin = space > 0 ? trimmed.substring(0, space) : trimmed;
  String path = space > 0 ? trimmed.substring(space + 1) : "last";
  pin.trim();
  path.trim();
  if (pin.isEmpty()) {
    writeLine("Usage: nfc clone <pin> [dump_path|last]");
    return;
  }
  writeLine("Hold blank Type 2 tag near PN532 (clone user pages)...");
  const NfcCloneResult result = NfcClone.restoreDump(path.c_str(), pin.c_str(), 4);
  write(NfcClone.toText(result).c_str());
}

void CliEngine::printNfcKill(const String& pin) {
  String trimmed = pin;
  trimmed.trim();
  if (trimmed.isEmpty()) {
    writeLine("Usage: nfc kill <pin>");
    return;
  }
  writeLine("Hold Type 2 tag near PN532 (PERMANENT lock)...");
  const NfcKillResult result = NfcKill.killTag(trimmed.c_str());
  write(NfcKill.toText(result).c_str());
}

void CliEngine::printNfcClassicDump(const String& args) {
  String key = args;
  key.trim();
  if (key.isEmpty()) {
    key = "FFFFFFFFFFFF";
  }
  writeLine("Hold Mifare Classic card near PN532...");
  const NfcClassicResult result = NfcClassic.dumpWithKey(key.c_str());
  write(NfcClassic.dumpToText(result).c_str());
}

void CliEngine::printNfcClassicClone(const String& args) {
  String trimmed = args;
  trimmed.trim();
  const int space = trimmed.indexOf(' ');
  String pin = space > 0 ? trimmed.substring(0, space) : trimmed;
  String key = space > 0 ? trimmed.substring(space + 1) : "";
  pin.trim();
  key.trim();
  if (pin.isEmpty()) {
    writeLine("Usage: nfc classic clone <pin> [key]");
    return;
  }
  writeLine("Hold destination Classic card near PN532...");
  const NfcClassicResult result = NfcClassic.cloneLast(key.c_str(), pin.c_str());
  write(NfcClassic.dumpToText(result).c_str());
}

void CliEngine::printNfcKeysScan(const String& args) {
  String trimmed = args;
  trimmed.trim();
  const uint8_t sector = trimmed.isEmpty() ? 0 : static_cast<uint8_t>(trimmed.toInt());
  writeLine("Hold Mifare Classic card near PN532 (key scan)...");
  const NfcKeyScanResult result = NfcClassic.scanKey(sector);
  write(NfcClassic.keyScanToText(result).c_str());
}

void CliEngine::printConfig() {
  if (!_ctx) {
    return;
  }

  writeLine("── Current Config ──");
  printf("Name:      %s\n", _ctx->config.ownerName);
  printf("Title:     %s\n", _ctx->config.title);
  printf("Email:     %s\n", _ctx->config.email);
  printf("Website:   %s\n", _ctx->config.website);
  printf("LinkedIn:  %s\n", _ctx->config.linkedin);
  printf("Source:    %s\n", Config.getSourceLabel());
}

void CliEngine::printUnknown(const String& cmd) {
  printf("Unknown command: %s\n", cmd.c_str());
  writeLine("Type 'help' for available commands.");
}

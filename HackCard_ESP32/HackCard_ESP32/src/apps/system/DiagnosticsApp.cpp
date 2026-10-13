#include "DiagnosticsApp.h"

#include "config/board_config.h"
#include "../../platform/hal/HalBuzzer.h"
#include "../../platform/hal/HalRgbRing.h"
#include "../../platform/hal/HalWifiAp.h"
#include "../../platform/hal/HalWifiLed.h"
#include "../../platform/hal/HalNfc.h"
#include "../../core/StorageManager.h"

DiagnosticsApp Diagnostics;

void DiagnosticsApp::addItem(DiagReport& report, const char* name, bool pass, const String& detail) const {
  if (report.count >= sizeof(report.items) / sizeof(report.items[0])) {
    return;
  }
  report.items[report.count].name = name;
  report.items[report.count].pass = pass;
  report.items[report.count].detail = detail;
  report.count++;
}

DiagReport DiagnosticsApp::run(bool peripheralsReady) {
  DiagReport report;

  addItem(report, "Storage",
          Storage.isFlashReady(),
          Storage.getStatusLine());

  addItem(report, "SD Card",
          true,
          Storage.isSdReady() ? "SD detected — Full Mode" : "No SD — Flash Mode (OK)");

  addItem(report, "Wi-Fi AP",
          WifiAp.verifyRunning(),
          WifiAp.verifyRunning() ? WifiAp.getStatusText() : WifiAp.getLastError());

  if (peripheralsReady) {
    RgbRing.showColor(0, 40, 0);
    delay(120);
    RgbRing.showColor(0, 0, 40);
    delay(120);
    RgbRing.setPattern(RingPattern::Idle);
    addItem(report, "RGB Ring", true, "12-LED ring cycled green/blue");

    WifiLed.setState(WifiLedState::Scanning);
    delay(150);
    WifiLed.setState(WifiLedState::ApActive);
    addItem(report, "WiFi LED", true, "Status LED animated");

    if (Buzzer.isEnabled()) {
      Buzzer.playSuccess();
      addItem(report, "Buzzer", true, "Success tone played");
    } else {
      addItem(report, "Buzzer", true, "Disabled in settings (skipped)");
    }
  } else {
    addItem(report, "RGB Ring", false, "Peripherals not ready yet — wait 3s after boot");
    addItem(report, "WiFi LED", false, "Peripherals not ready yet");
    addItem(report, "Buzzer", false, "Peripherals not ready yet");
  }

#if APP_NFC_READ
  addItem(report, "NFC (PN532)", Nfc.isReady() || Nfc.begin(), Nfc.getChipInfo());
#else
  addItem(report, "NFC (PN532)", false, "NFC app disabled in app_registry.h");
#endif
  addItem(report, "BOOT Button", true, "Manual — short press should move ring pointer");

  report.allPass = true;
  for (uint8_t i = 0; i < report.count; i++) {
    if (!report.items[i].pass && report.items[i].name != "NFC (PN532)" && report.items[i].name != "BOOT Button") {
      report.allPass = false;
      break;
    }
  }

  return report;
}

String DiagnosticsApp::toJson(const DiagReport& report) const {
  String json = "{\"all_pass\":";
  json += report.allPass ? "true" : "false";
  json += ",\"items\":[";
  for (uint8_t i = 0; i < report.count; i++) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"name\":\"";
    json += report.items[i].name;
    json += "\",\"pass\":";
    json += report.items[i].pass ? "true" : "false";
    json += ",\"detail\":\"";
    json += report.items[i].detail;
    json += "\"}";
  }
  json += "]}";
  return json;
}

String DiagnosticsApp::toText(const DiagReport& report) const {
  String text = "── Hardware Diagnostics ──\n";
  for (uint8_t i = 0; i < report.count; i++) {
    text += report.items[i].pass ? "[PASS] " : "[FAIL] ";
    text += report.items[i].name;
    text += " — ";
    text += report.items[i].detail;
    text += "\n";
  }
  text += report.allPass ? "Overall: PASS\n" : "Overall: ISSUES FOUND\n";
  return text;
}

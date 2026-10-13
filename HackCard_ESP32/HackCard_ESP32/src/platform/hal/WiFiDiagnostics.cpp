#include "WiFiDiagnostics.h"

#include "../../core/HcSerial.h"

#include "config/board_config.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace {

void printLine(const char* label, const char* value) {
  HcSerial.printf("[wifi-diag] %-18s %s\n", label, value);
}

void printLineInt(const char* label, int value) {
  HcSerial.printf("[wifi-diag] %-18s %d\n", label, value);
}

}  // namespace

const char* WifiDiagEspErrToString(int err) {
  return esp_err_to_name(static_cast<esp_err_t>(err));
}

void WifiDiagPrintBootReport() {
  HcSerial.println();
  HcSerial.println("[wifi-diag] === HackCard Wi-Fi Hardware Report ===");

  printLine("MCU", BOARD_MCU);
  printLineInt("RGB ring GPIO", PIN_RGB_RING);
  printLineInt("WiFi LED GPIO", PIN_RGB_WIFI);
  printLineInt("Buzzer GPIO", PIN_BUZZER);
  printLineInt("NFC SDA GPIO", PIN_NFC_SDA);
  printLineInt("SD CLK GPIO", PIN_SD_CLK);

  if (PIN_RGB_RING == 45) {
    HcSerial.println("[wifi-diag] NOTE: GPIO45 is an ESP32-S3 strapping pin.");
    HcSerial.println("[wifi-diag]       WS2812 on GPIO45 can affect Wi-Fi on custom PCBs.");
  }
  if (PIN_BUZZER == 37) {
    HcSerial.println("[wifi-diag] NOTE: LEDC/PWM on GPIO37 can interfere with Wi-Fi AP.");
    HcSerial.println("[wifi-diag]       Buzzer now detaches PWM when idle.");
  }

  wifi_mode_t mode = WIFI_MODE_NULL;
  const esp_err_t modeErr = esp_wifi_get_mode(&mode);
  printLine("esp_wifi_get_mode", WifiDiagEspErrToString(modeErr));
  printLineInt("wifi_mode", mode);

  wifi_config_t apConfig{};
  const esp_err_t cfgErr = esp_wifi_get_config(WIFI_IF_AP, &apConfig);
  printLine("esp_wifi_get_config", WifiDiagEspErrToString(cfgErr));
  if (cfgErr == ESP_OK) {
    HcSerial.printf("[wifi-diag] ap_ssid_from_hw   %s\n", reinterpret_cast<const char*>(apConfig.ap.ssid));
    printLineInt("ap_channel", apConfig.ap.channel);
    printLineInt("ap_max_conn", apConfig.ap.max_connection);
  }

  HcSerial.printf("[wifi-diag] softAPIP           %s\n", WiFi.softAPIP().toString().c_str());
  HcSerial.printf("[wifi-diag] softAPmac          %s\n", WiFi.softAPmacAddress().c_str());
  printLineInt("softAPstations", WiFi.softAPgetStationNum());

  HcSerial.println("[wifi-diag] =====================================");
  HcSerial.println();
}

void WifiDiagPrintRuntimeReport() {
  WifiDiagPrintBootReport();
}

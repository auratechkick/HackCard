#include "HalWifiAp.h"

#include "../../core/HcSerial.h"
#include "WiFiDiagnostics.h"
#include "config/board_config.h"

#include <esp_wifi.h>

HalWifiAp WifiAp;

namespace {

constexpr int kMaxConnections = 8;
constexpr int kFallbackChannels[] = {1, 6, 11};

bool waitForApIp(uint32_t timeoutMs) {
  const uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (WiFi.softAPIP() != IPAddress(0, 0, 0, 0)) {
      return true;
    }
    delay(50);
  }
  return WiFi.softAPIP() != IPAddress(0, 0, 0, 0);
}

void logWifi(const char* message) {
  HcSerial.printf("[wifi] %s\n", message);
}

}  // namespace

void HalWifiAp::preparePinsForWifi() {
  preparePeripheralPins();
}

void HalWifiAp::preparePeripheralPins() {
  // Keep HackCard peripherals high-Z until Wi-Fi is stable.
  // GPIO45 is a strapping pin; do not drive WS2812 before Wi-Fi starts.
  pinMode(PIN_RGB_RING, INPUT);
  pinMode(PIN_RGB_WIFI, INPUT);
  pinMode(PIN_BUZZER, INPUT);
  pinMode(PIN_SD_CS, INPUT);
  pinMode(PIN_NFC_SDA, INPUT_PULLUP);
  pinMode(PIN_NFC_SCL, INPUT_PULLUP);
}

bool HalWifiAp::begin(const char* ssid, const char* password, int channel) {
  _ssid = ssid ? ssid : "HackCard";
  _password = password ? password : "";
  _channel = (channel > 0) ? channel : 1;
  _lastError = "";

  preparePinsForWifi();

  logWifi("Starting access point (HackCard safe boot)...");
  HcSerial.printf("[wifi] SSID: %s (2.4 GHz only)\n", _ssid.c_str());
  HcSerial.printf("[wifi] Password: %s\n", _password.c_str());

  if (tryChannels(_ssid.c_str(), _password.c_str(), _channel)) {
    _running = true;
    HcSerial.println(getStatusText());
    WifiDiagPrintBootReport();
    return true;
  }

  logWifi("Secure AP failed, retrying open network for recovery...");
  if (tryChannels(_ssid.c_str(), nullptr, _channel)) {
    _running = true;
    _lastError = "Started open AP fallback";
    HcSerial.println(getStatusText());
    WifiDiagPrintBootReport();
    return true;
  }

  _running = false;
  _lastError = "Could not start access point on any channel";
  logWifi("ERROR: Could not start access point");
  WifiDiagPrintBootReport();
  return false;
}

bool HalWifiAp::tryChannels(const char* ssid, const char* password, int preferredChannel) {
  if (startAp(ssid, password, preferredChannel)) {
    return true;
  }

  for (int fallbackChannel : kFallbackChannels) {
    if (fallbackChannel == preferredChannel) {
      continue;
    }
    HcSerial.printf("[wifi] Retrying AP on channel %d...\n", fallbackChannel);
    if (startAp(ssid, password, fallbackChannel)) {
      _channel = fallbackChannel;
      return true;
    }
  }
  return false;
}

bool HalWifiAp::startAp(const char* ssid, const char* password, int channel) {
  WiFi.persistent(false);
  WiFi.setSleep(WIFI_PS_NONE);

  // Softer reset — full erase can break calibration on some modules.
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(150);

  // AP+STA keeps the soft AP up while the STA interface runs background scans.
  if (!WiFi.mode(WIFI_AP_STA)) {
    _lastError = "WiFi.mode(WIFI_AP_STA) failed";
    logWifi(_lastError.c_str());
    return false;
  }

  delay(100);

  wifi_country_t country = {};
  country.schan = 1;
  country.nchan = 13;
  country.policy = WIFI_COUNTRY_POLICY_AUTO;
  esp_wifi_set_country(&country);

  WiFi.setTxPower(WIFI_POWER_15dBm);

  const IPAddress apIp(192, 168, 4, 1);
  const IPAddress gateway(192, 168, 4, 1);
  const IPAddress subnet(255, 255, 255, 0);
  if (!WiFi.softAPConfig(apIp, gateway, subnet)) {
    logWifi("softAPConfig failed (continuing anyway)");
  }

  bool started = false;
  if (password && strlen(password) >= 8) {
    started = WiFi.softAP(ssid, password, channel, 0, kMaxConnections);
  } else if (password && strlen(password) > 0) {
    HcSerial.println("[wifi] Password too short (min 8). Using open AP.");
    started = WiFi.softAP(ssid, nullptr, channel, 0, kMaxConnections);
  } else {
    started = WiFi.softAP(ssid, nullptr, channel, 0, kMaxConnections);
  }

  if (!started) {
    _lastError = "WiFi.softAP returned false";
    return false;
  }

  if (!waitForApIp(5000)) {
    _lastError = "AP started but IP was not assigned";
    logWifi(_lastError.c_str());
    return false;
  }

  _channel = channel;
  HcSerial.printf("[wifi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
  HcSerial.printf("[wifi] AP MAC: %s\n", WiFi.softAPmacAddress().c_str());
  HcSerial.printf("[wifi] AP channel: %d\n", channel);
  return true;
}

bool HalWifiAp::verifyRunning() const {
  return _running && (WiFi.getMode() & WIFI_MODE_AP) && (WiFi.softAPIP() != IPAddress(0, 0, 0, 0));
}

bool HalWifiAp::restart() {
  logWifi("Restarting AP...");
  _running = false;
  return begin(_ssid.c_str(), _password.c_str(), _channel);
}

bool HalWifiAp::ensureSoftApWithStaConnected() {
  if (_ssid.isEmpty()) {
    _ssid = "HackCard";
  }
  WiFi.persistent(false);
  WiFi.setSleep(WIFI_PS_NONE);

  if (WiFi.getMode() != WIFI_AP_STA) {
    WiFi.mode(WIFI_AP_STA);
    delay(100);
  }

  if (WiFi.softAPIP() == IPAddress(0, 0, 0, 0)) {
    const IPAddress apIp(192, 168, 4, 1);
    const IPAddress gateway(192, 168, 4, 1);
    const IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIp, gateway, subnet);
    const char* pass = (_password.length() >= 8) ? _password.c_str() : nullptr;
    if (!WiFi.softAP(_ssid.c_str(), pass, _channel > 0 ? _channel : 1, 0, kMaxConnections)) {
      _lastError = "softAP alongside STA failed";
      logWifi(_lastError.c_str());
      return false;
    }
    HcSerial.printf("[wifi] AP+STA: soft-AP restored @ %s\n", WiFi.softAPIP().toString().c_str());
  }

  _running = true;
  return verifyRunning();
}

void HalWifiAp::maintain() {
  if (!_running) {
    return;
  }

  const uint32_t now = millis();
  if (now - _lastMaintainMs < 5000) {
    return;
  }
  _lastMaintainMs = now;

  WiFi.setSleep(WIFI_PS_NONE);

  if (!verifyRunning()) {
    logWifi("AP dropped — attempting restart");
    restart();
  }
}

String HalWifiAp::getIp() const {
  return WiFi.softAPIP().toString();
}

String HalWifiAp::getStatusText() const {
  String text = "WiFi AP: ";
  text += verifyRunning() ? "ON" : "OFF";
  text += " | SSID: ";
  text += _ssid;
  text += " | Ch: ";
  text += String(_channel);
  text += " | IP: ";
  text += getIp();
  if (!_lastError.isEmpty()) {
    text += " | Last: ";
    text += _lastError;
  }
  return text;
}

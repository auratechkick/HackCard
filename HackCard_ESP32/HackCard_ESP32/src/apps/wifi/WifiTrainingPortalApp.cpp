#include "WifiTrainingPortalApp.h"

#include "config/app_registry.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalBuzzer.h"
#if APP_WIFI_PORTAL
#include "../../platform/hal/WifiCaptivePortal.h"
#endif

WifiTrainingPortalApp WifiTraining;

namespace {

constexpr char kConfigPath[] = "/wifi/training_portal.txt";
constexpr char kCapturesPath[] = "/wifi/training_captures.log";

const char* templateName(uint8_t id) {
  switch (id) {
    case 0:
      return "wifi_login";
    case 1:
      return "network_auth";
    case 2:
      return "hotel_wifi";
    case 3:
      return "firmware_update";
    case 4:
      return "plugin_update";
    case 5:
      return "oauth_login";
    case 6:
      return "isp_login";
    default:
      return "wifi_login";
  }
}

bool containsIgnoreCase(const char* haystack, const char* needle) {
  if (!haystack || !needle) {
    return false;
  }
  String h = haystack;
  String n = needle;
  h.toLowerCase();
  n.toLowerCase();
  return h.indexOf(n) >= 0;
}

String escapeJson(const char* text) {
  String out;
  if (!text) {
    return out;
  }
  for (const char* p = text; *p; ++p) {
    if (*p == '\\' || *p == '"') {
      out += '\\';
    }
    if (*p == '\n' || *p == '\r') {
      continue;
    }
    out += *p;
  }
  return out;
}

}  // namespace

void WifiTrainingPortalApp::begin() {
  loadConfig();
  syncCapturesFromStorage();
}

void WifiTrainingPortalApp::syncCapturesFromStorage() {
  String text;
  if (!Storage.readTextFile(kCapturesPath, text) || text.isEmpty()) {
    _memoryCaptures = "";
    _captureCount = 0;
    return;
  }
  _memoryCaptures = text;
  _captureCount = 0;
  for (size_t i = 0; i < text.length(); ++i) {
    if (text.charAt(i) == '\n') {
      _captureCount++;
    }
  }
  if (!text.endsWith("\n")) {
    _captureCount++;
  }
}

bool WifiTrainingPortalApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}

void WifiTrainingPortalApp::applyTemplateDefaults() {
  switch (_templateId) {
    case 1:
      strlcpy(_headline, "Network Authentication", sizeof(_headline));
      strlcpy(_subtitle, "Your organization requires sign-in before internet access.", sizeof(_subtitle));
      break;
    case 2:
      strlcpy(_headline, "Welcome — Hotel Wi-Fi", sizeof(_headline));
      strlcpy(_subtitle, "Enter your room number and last name to connect.", sizeof(_subtitle));
      break;
    case 3:
      strlcpy(_headline, "Router Firmware Update", sizeof(_headline));
      strlcpy(_subtitle, "A critical security update is available. Enter admin credentials to continue.", sizeof(_subtitle));
      break;
    case 4:
      strlcpy(_headline, "Browser Plugin Required", sizeof(_headline));
      strlcpy(_subtitle, "Install the security plugin to continue browsing safely.", sizeof(_subtitle));
      break;
    case 5:
      strlcpy(_headline, "Sign in to continue", sizeof(_headline));
      strlcpy(_subtitle, "Use your account to access this network.", sizeof(_subtitle));
      break;
    case 6:
      strlcpy(_headline, "Internet service sign-in", sizeof(_headline));
      strlcpy(_subtitle, "Verify your account to restore internet access.", sizeof(_subtitle));
      break;
    default:
      strlcpy(_headline, "Sign in to Wi-Fi", sizeof(_headline));
      strlcpy(_subtitle, "Enter your details to access the internet.", sizeof(_subtitle));
      break;
  }
}

bool WifiTrainingPortalApp::saveConfig() const {
  Storage.ensureDirectory("/wifi");
  String text;
  text += "template=";
  text += String(_templateId);
  text += "\nnetwork=";
  text += _networkName;
  text += "\nheadline=";
  text += _headline;
  text += "\nsubtitle=";
  text += _subtitle;
  return Storage.writeTextFile(kConfigPath, text);
}

bool WifiTrainingPortalApp::loadConfig() {
  String text;
  if (!Storage.readTextFile(kConfigPath, text)) {
    applyTemplateDefaults();
    return false;
  }

  int start = 0;
  while (start < text.length()) {
    int end = text.indexOf('\n', start);
    if (end < 0) {
      end = text.length();
    }
    String line = text.substring(start, end);
    line.trim();
    const int eq = line.indexOf('=');
    if (eq > 0) {
      const String key = line.substring(0, eq);
      const String val = line.substring(eq + 1);
      if (key == "template") {
        _templateId = static_cast<uint8_t>(val.toInt());
      } else if (key == "network") {
        strlcpy(_networkName, val.c_str(), sizeof(_networkName));
      } else if (key == "headline") {
        strlcpy(_headline, val.c_str(), sizeof(_headline));
      } else if (key == "subtitle") {
        strlcpy(_subtitle, val.c_str(), sizeof(_subtitle));
      }
    }
    start = end + 1;
  }

  if (_headline[0] == '\0' || _subtitle[0] == '\0') {
    applyTemplateDefaults();
  }
  return true;
}

uint8_t WifiTrainingPortalApp::suggestTemplateForSsid(const char* ssid) {
  if (!ssid || ssid[0] == '\0') {
    return 0;
  }
  if (containsIgnoreCase(ssid, "xfinity") || containsIgnoreCase(ssid, "comcast") ||
      containsIgnoreCase(ssid, "att") || containsIgnoreCase(ssid, "verizon") ||
      containsIgnoreCase(ssid, "spectrum") || containsIgnoreCase(ssid, "cox") ||
      containsIgnoreCase(ssid, "isp")) {
    return 6;
  }
  if (containsIgnoreCase(ssid, "hotel") || containsIgnoreCase(ssid, "inn") ||
      containsIgnoreCase(ssid, "marriott") || containsIgnoreCase(ssid, "hilton")) {
    return 2;
  }
  if (containsIgnoreCase(ssid, "corp") || containsIgnoreCase(ssid, "enterprise") ||
      containsIgnoreCase(ssid, "office")) {
    return 1;
  }
  if (containsIgnoreCase(ssid, "guest") || containsIgnoreCase(ssid, "free")) {
    return 0;
  }
  if (containsIgnoreCase(ssid, "router") || containsIgnoreCase(ssid, "firmware") ||
      containsIgnoreCase(ssid, "asus") || containsIgnoreCase(ssid, "netgear")) {
    return 3;
  }
  return 0;
}

bool WifiTrainingPortalApp::applyAutoTemplateForSsid(const char* ssid) {
  if (!ssid || ssid[0] == '\0') {
    return false;
  }
  const uint8_t templateId = suggestTemplateForSsid(ssid);
  setTemplate(templateId);
  setNetworkName(ssid);
  return true;
}

bool WifiTrainingPortalApp::setTemplate(uint8_t templateId) {
  if (templateId > 6) {
    return false;
  }
  _templateId = templateId;
  applyTemplateDefaults();
  return saveConfig();
}

bool WifiTrainingPortalApp::setNetworkName(const char* name) {
  if (!name || name[0] == '\0' || strlen(name) > 31) {
    return false;
  }
  strlcpy(_networkName, name, sizeof(_networkName));
  return saveConfig();
}

bool WifiTrainingPortalApp::setHeadline(const char* headline) {
  if (!headline || strlen(headline) > 63) {
    return false;
  }
  strlcpy(_headline, headline, sizeof(_headline));
  return saveConfig();
}

bool WifiTrainingPortalApp::setSubtitle(const char* subtitle) {
  if (!subtitle || strlen(subtitle) > 127) {
    return false;
  }
  strlcpy(_subtitle, subtitle, sizeof(_subtitle));
  return saveConfig();
}

WifiTrainingResult WifiTrainingPortalApp::start(const char* labPin) {
  WifiTrainingResult result;

  if (_active) {
    strncpy(result.error, "Training portal already active", sizeof(result.error) - 1);
    return result;
  }

  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }

  saveConfig();
#if APP_WIFI_PORTAL
  _savedPortalEnabled = WifiCaptive.isEnabled();
  WifiCaptive.setEnabled(true);
#endif

  _active = true;
  if (Buzzer.isEnabled()) {
    Buzzer.playNfcReady();
  }
  Log.activity("wifi", "training_start", templateName(_templateId));

  result.success = true;
  result.active = true;
  return result;
}

WifiTrainingResult WifiTrainingPortalApp::stop() {
  WifiTrainingResult result;

  if (!_active) {
    result.success = true;
    result.active = false;
    return result;
  }

  _active = false;
#if APP_WIFI_PORTAL
  WifiCaptive.setEnabled(_savedPortalEnabled);
#endif

  if (Buzzer.isEnabled()) {
    Buzzer.playSuccess();
  }
  Log.activity("wifi", "training_stop", "ok");

  result.success = true;
  result.active = false;
  return result;
}

bool WifiTrainingPortalApp::recordCapture(const char* username, const char* password) {
  Storage.ensureDirectory("/wifi");

  String line;
  line += "{\"t\":";
  line += String(millis() / 1000);
  line += ",\"template\":\"";
  line += templateName(_templateId);
  line += "\",\"user\":\"";
  line += escapeJson(username);
  line += "\",\"pass\":\"";
  line += escapeJson(password);
  line += "\"}\n";

  _memoryCaptures += line;
  while (_memoryCaptures.length() > 6000) {
    const int cut = _memoryCaptures.indexOf('\n');
    if (cut < 0) {
      _memoryCaptures = "";
      break;
    }
    _memoryCaptures = _memoryCaptures.substring(cut + 1);
  }

  const bool stored =
      Storage.appendBinaryFile(kCapturesPath, reinterpret_cast<const uint8_t*>(line.c_str()), line.length());
  if (!stored) {
    Log.warn("wifi", "training capture RAM-only (flash write failed)");
  }

  _captureCount++;
  Log.activity("wifi", "training_capture", username ? username : "anonymous");
  return true;
}

String WifiTrainingPortalApp::getCapturesTail(uint8_t maxLines) const {
  String text = _memoryCaptures;
  if (text.isEmpty() && (!Storage.readTextFile(kCapturesPath, text) || text.isEmpty())) {
    return "(no captures yet)";
  }

  int count = 0;
  int lastStart = text.length();
  for (int i = text.length() - 1; i >= 0; --i) {
    if (text.charAt(i) == '\n') {
      count++;
      if (count == static_cast<int>(maxLines)) {
        lastStart = i + 1;
        break;
      }
    }
  }
  if (count < static_cast<int>(maxLines) || lastStart >= static_cast<int>(text.length())) {
    lastStart = 0;
  }
  return text.substring(lastStart);
}

String WifiTrainingPortalApp::getAllCapturesText() const {
  String disk;
  if (Storage.readTextFile(kCapturesPath, disk) && disk.length() > 0) {
    if (_memoryCaptures.length() > disk.length()) {
      return _memoryCaptures;
    }
    return disk;
  }
  return _memoryCaptures;
}

bool WifiTrainingPortalApp::exportCapturesToSd(char* outPath, size_t outPathLen, char* error,
                                               size_t errorLen) const {
  if (!outPath || outPathLen == 0) {
    if (error && errorLen > 0) {
      strncpy(error, "Invalid export path buffer", errorLen - 1);
    }
    return false;
  }
  outPath[0] = '\0';

  if (!Storage.isSdReady()) {
    if (error && errorLen > 0) {
      strncpy(error, "Insert an SD card (Full Mode)", errorLen - 1);
    }
    return false;
  }

  const String data = getAllCapturesText();
  if (data.isEmpty()) {
    if (error && errorLen > 0) {
      strncpy(error, "No captures to export", errorLen - 1);
    }
    return false;
  }

  Storage.ensureDirectory("/wifi/exports");
  snprintf(outPath, outPathLen, "/wifi/exports/training_%lu.log", static_cast<unsigned long>(millis() / 1000UL));

  if (!Storage.writeTextFileSdOnly(outPath, data)) {
    if (error && errorLen > 0) {
      strncpy(error, "Could not write export file on SD", errorLen - 1);
    }
    outPath[0] = '\0';
    return false;
  }

  Log.activity("wifi", "training_export_sd", outPath);
  return true;
}

bool WifiTrainingPortalApp::clearCaptures() {
  _captureCount = 0;
  _memoryCaptures = "";
  return Storage.writeTextFile(kCapturesPath, "");
}

String WifiTrainingPortalApp::buildPortalHtml(const String& ip) const {
  String html;
  html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>";
  html += _networkName;
  html += "</title><style>";
  html += "body{margin:0;font-family:Segoe UI,Arial,sans-serif;background:#eef2f7;color:#1f2937;min-height:100vh;}";
  html += ".banner{background:#7c2d12;color:#fff;text-align:center;padding:10px 14px;font-size:13px;font-weight:700;}";
  html += ".wrap{max-width:400px;margin:0 auto;padding:24px 18px 40px;}";
  html += ".card{background:#fff;border:1px solid #d1d5db;border-radius:12px;padding:22px;box-shadow:0 8px 24px rgba(0,0,0,.08);}";
  html += ".net{font-size:12px;color:#6b7280;text-transform:uppercase;letter-spacing:.08em;margin-bottom:8px;}";
  html += "h1{margin:0 0 8px;font-size:22px;}";
  html += ".sub{color:#6b7280;font-size:14px;line-height:1.45;margin-bottom:18px;}";
  html += "label{display:block;font-size:13px;margin:12px 0 4px;color:#374151;}";
  html += "input{width:100%;box-sizing:border-box;border:1px solid #cbd5e1;border-radius:8px;padding:11px 12px;font-size:15px;}";
  html += "button{width:100%;margin-top:16px;background:#2563eb;color:#fff;border:0;border-radius:8px;padding:12px;font-size:15px;font-weight:700;cursor:pointer;}";
  html += ".foot{margin-top:16px;font-size:11px;color:#9ca3af;text-align:center;}";
  html += "</style></head><body>";
  html += "<div class='banner'>TRAINING SIMULATION — Authorized security awareness lab only</div>";
  html += "<div class='wrap'><div class='card'>";
  html += "<div class='net'>Network: ";
  html += _networkName;
  html += "</div><h1>";
  html += _headline;
  html += "</h1><p class='sub'>";
  html += _subtitle;
  html += "</p><form method='POST' action='/api/wifi/training/submit'>";

  if (_templateId == 2) {
    html += "<label>Room number</label><input name='username' required autocomplete='off'>";
    html += "<label>Last name</label><input name='password' type='text' required autocomplete='off'>";
  } else if (_templateId == 3) {
    html += "<label>Router model</label><input name='username' placeholder='e.g. TP-Link Archer' required autocomplete='off'>";
    html += "<label>Admin password</label><input name='password' type='password' required autocomplete='off'>";
  } else if (_templateId == 4) {
    html += "<label>Email address</label><input name='username' type='email' required autocomplete='off'>";
    html += "<label>Confirm to install</label><input name='password' type='text' placeholder='Type INSTALL' required autocomplete='off'>";
  } else if (_templateId == 5) {
    html += "<p style='font-size:13px;color:#6b7280;margin-bottom:12px'>Continue with your account</p>";
    html += "<label>Email</label><input name='username' type='email' required autocomplete='off'>";
    html += "<label>Password</label><input name='password' type='password' required autocomplete='off'>";
    html += "<button type='submit' style='background:#4285f4;margin-top:12px'>Sign in</button>";
    html += "<input type='hidden' name='_oauth' value='1'>";
  } else if (_templateId == 6) {
    html += "<label>Account number or email</label><input name='username' required autocomplete='off'>";
    html += "<label>Account password</label><input name='password' type='password' required autocomplete='off'>";
  } else {
    html += "<label>Email or username</label><input name='username' required autocomplete='off'>";
    html += "<label>Password</label><input name='password' type='password' required autocomplete='off'>";
  }

  if (_templateId != 5) {

    html += "<button type='submit'>Continue</button>";
  }
  html += "</form>";
  html += "<p class='foot'>HackCard training portal · ";
  html += ip;
  html += "</p></div></div></body></html>";
  return html;
}

String WifiTrainingPortalApp::buildRevealHtml(const String& ip) const {
  String html;
  html += "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Training Complete</title><style>";
  html += "body{margin:0;font-family:Segoe UI,Arial,sans-serif;background:#0f172a;color:#e2e8f0;min-height:100vh;}";
  html += ".wrap{max-width:440px;margin:0 auto;padding:32px 18px;text-align:center;}";
  html += ".icon{font-size:48px;margin-bottom:12px;}";
  html += "h1{margin:0 0 10px;font-size:24px;color:#fbbf24;}";
  html += "p{font-size:15px;line-height:1.55;color:#cbd5e1;margin:10px 0;}";
  html += ".box{background:#1e293b;border:1px solid #334155;border-radius:12px;padding:16px;text-align:left;margin-top:18px;}";
  html += ".box li{margin:8px 0;}";
  html += "a{color:#60a5fa;text-decoration:none;}";
  html += "</style></head><body><div class='wrap'>";
  html += "<div class='icon'>!</div>";
  html += "<h1>This Was a Simulation</h1>";
  html += "<p>You just interacted with a <strong>training captive portal</strong> on HackCard.</p>";
  html += "<div class='box'><p><strong>In a real attack:</strong></p><ul>";
  html += "<li>Credentials could be stolen on fake Wi-Fi</li>";
  html += "<li>Portals can mimic hotels, cafés, or office networks</li>";
  html += "<li>Always verify HTTPS and avoid entering passwords on unknown hotspots</li>";
  html += "</ul></div>";
  html += "<p>Your entries were saved on the HackCard device for instructor debrief (Training → Capture log).</p>";
  html += "<p><a href='http://";
  html += ip;
  html += "/wifi/training'>Open Training Dashboard</a></p>";
  html += "</div></body></html>";
  return html;
}

String WifiTrainingPortalApp::statusToJson() const {
  String json = "{\"active\":";
  json += _active ? "true" : "false";
  json += ",\"template\":\"";
  json += templateName(_templateId);
  json += "\",\"network_name\":\"";
  json += escapeJson(_networkName);
  json += "\",\"capture_count\":";
  json += String(_captureCount);
  json += ",\"sd_ready\":";
  json += Storage.isSdReady() ? "true" : "false";
  json += "}";
  return json;
}

String WifiTrainingPortalApp::configToJson() const {
  String json = "{\"template\":\"";
  json += templateName(_templateId);
  json += "\",\"template_id\":";
  json += String(_templateId);
  json += ",\"network_name\":\"";
  json += escapeJson(_networkName);
  json += "\",\"headline\":\"";
  json += escapeJson(_headline);
  json += "\",\"subtitle\":\"";
  json += escapeJson(_subtitle);
  json += "\"}";
  return json;
}

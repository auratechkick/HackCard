#include "HidLabApp.h"

#include "HidDuckyEngine.h"
#include "HidPayloadStore.h"
#include "../../core/HcSerial.h"
#include "../../core/AppContext.h"
#include "../../core/LabPin.h"
#include "../../core/Logger.h"
#include "../../core/StorageManager.h"
#include "../../platform/hal/HalUsbHid.h"

#include <cstdlib>
#include <cstring>



HidLabApp HidLab;



namespace {



void hidSerial(const char* message) {

  HcSerial.printf("[hid] %s\n", message);

}



bool actionNeedsPayload(HidAction action) {

  return action == HidAction::TypeText || action == HidAction::OpenWebsite ||
         action == HidAction::UnlockScreen ||
         action == HidAction::KeyPress || action == HidAction::MouseMove;

}



const char* payloadRequiredMessage(HidAction action) {

  switch (action) {

    case HidAction::OpenWebsite:

      return "URL required";

    case HidAction::UnlockScreen:

      return "Host password required";

    case HidAction::KeyPress:

      return "Key name required";

    case HidAction::MouseMove:

      return "Move delta required (dx,dy)";

    case HidAction::TypeText:

    default:

      return "Text required";

  }

}



}  // namespace



bool HidLabApp::begin() {

  _ready = UsbHid.begin();

  if (!_ready) {

    strncpy(_lastMessage, UsbHid.lastError(), sizeof(_lastMessage) - 1);

    hidSerial(_lastMessage);

  } else if (UsbHid.isHostConnected()) {

    strncpy(_lastMessage, "USB host connected", sizeof(_lastMessage) - 1);

    hidSerial("USB host connected");

  } else {

    strncpy(_lastMessage, "HID ready, waiting for USB host", sizeof(_lastMessage) - 1);

    hidSerial("HID ready, waiting for USB host");

  }

  return _ready;

}



bool HidLabApp::checkPin(const char* labPin, char* error, size_t errorLen) const {
  return labPinCheck(labPin, error, errorLen);
}



HidTargetOs HidLabApp::getTargetOs(const HackCardConfig& config) const {

  if (config.hidTargetOs >= static_cast<uint8_t>(HidTargetOs::Count)) {

    return HidTargetOs::Windows;

  }

  return static_cast<HidTargetOs>(config.hidTargetOs);

}



bool HidLabApp::setTargetOs(HackCardConfig& config, HidTargetOs os) {

  if (static_cast<uint8_t>(os) >= static_cast<uint8_t>(HidTargetOs::Count)) {

    return false;

  }

  config.hidTargetOs = static_cast<uint8_t>(os);

  return true;

}



HidLabResult HidLabApp::status() const {

  HidLabResult result;

  result.success = _ready && UsbHid.isHostConnected();

  if (!_ready) {

    strncpy(result.error, UsbHid.lastError()[0] ? UsbHid.lastError() : "USB HID not initialized", sizeof(result.error));

  } else if (!UsbHid.isHostConnected()) {

    strncpy(result.error, "Plug HackCard into target PC via USB-C", sizeof(result.error));

  } else {

    strncpy(result.error, "Ready", sizeof(result.error));

  }

  return result;

}



String HidLabApp::statusToJson(HidTargetOs os) const {

  const HidLabResult st = status();

  String json = "{";

  json += "\"ready\":" + String(_ready ? "true" : "false") + ",";

  json += "\"host_connected\":" + String(st.success ? "true" : "false") + ",";

  json += "\"target_os\":\"" + String(hidOsName(os)) + "\",";

  json += "\"status\":\"" + String(st.error) + "\",";
  json += "\"last\":\"" + String(_lastMessage) + "\",";
  HidPayloadEntry entries[HidPayloadStore::kMaxEntries];
  json += "\"payload_count\":" + String(HidPayloads.listEntries(entries, HidPayloadStore::kMaxEntries)) + ",";
  json += "\"storage\":\"" + String(Storage.isSdReady() ? "sd" : "flash") + "\",";
  json += "\"button_armed\":" + String(_buttonArmed ? "true" : "false") + ",";
  json += "\"button_payload\":\"" + String(_buttonPayload) + "\",";
  json += "\"button_expires_ms\":" +
          String(_buttonArmed && millis() < _buttonArmExpiresMs ? static_cast<int32_t>(_buttonArmExpiresMs - millis()) : 0);
  json += "}";
  return json;
}

String HidLabApp::armStatusToJson() const {
  String json = "{";
  json += "\"button_armed\":" + String(_buttonArmed ? "true" : "false") + ",";
  json += "\"button_payload\":\"" + String(_buttonPayload) + "\",";
  json += "\"button_expires_ms\":" +
          String(_buttonArmed && millis() < _buttonArmExpiresMs ? static_cast<int32_t>(_buttonArmExpiresMs - millis()) : 0);
  json += "}";
  return json;
}



bool HidLabApp::execute(HidTargetOs os, HidAction action, const char* payload) {

  switch (action) {

    case HidAction::TypeText:

      return UsbHid.typeTextPayload(payload);

    case HidAction::OpenWebsite:

      return UsbHid.openWebsite(os, payload);

    case HidAction::LockScreen:

      return UsbHid.lockScreen(os);

    case HidAction::UnlockScreen:

      return UsbHid.unlockScreen(os, payload);

    case HidAction::Calculator:

      return UsbHid.openCalculator(os);

    case HidAction::Screenshot:

      return UsbHid.takeScreenshot(os);

    case HidAction::Terminal:

      return UsbHid.openTerminal(os);

    case HidAction::TerminalInfo:

      return UsbHid.runTerminalInfo(os);

    case HidAction::AppSwitcher:

      return UsbHid.appSwitcher(os);

    case HidAction::CloseWindow:

      return UsbHid.closeWindow(os);

    case HidAction::VolumeMute:

      return UsbHid.volumeMute();

    case HidAction::VolumeUp:

      return UsbHid.volumeUp();

    case HidAction::VolumeDown:

      return UsbHid.volumeDown();

    case HidAction::MediaPlayPause:

      return UsbHid.mediaPlayPause();

    case HidAction::MediaNext:

      return UsbHid.mediaNext();

    case HidAction::MediaPrevious:

      return UsbHid.mediaPrevious();

    case HidAction::MouseClick:

      return UsbHid.mouseClick(payload && payload[0] != '\0' ? payload : "left");

    case HidAction::MouseMove: {

      if (!payload || payload[0] == '\0') {

        return false;

      }

      int dx = 0;

      int dy = 0;

      const char* comma = strchr(payload, ',');

      if (!comma) {

        return false;

      }

      dx = atoi(payload);

      dy = atoi(comma + 1);

      if (dx > 127) {

        dx = 127;

      } else if (dx < -127) {

        dx = -127;

      }

      if (dy > 127) {

        dy = 127;

      } else if (dy < -127) {

        dy = -127;

      }

      return UsbHid.mouseMove(static_cast<int8_t>(dx), static_cast<int8_t>(dy));

    }

    case HidAction::KeyPress:

      return UsbHid.pressNamedKey(payload);

    case HidAction::Notepad:

    default:

      return UsbHid.openNotepad(os);

  }

}



HidLabResult HidLabApp::runAction(HidTargetOs os, HidAction action, const char* labPin, const char* payload) {

  HidLabResult result;



  HcSerial.printf("[hid] action=%s os=%s\n", hidActionName(action), hidOsName(os));



  if (!checkPin(labPin, result.error, sizeof(result.error))) {

    hidSerial(result.error);

    return result;

  }

  if (!_ready) {

    strncpy(result.error, UsbHid.lastError()[0] ? UsbHid.lastError() : "USB HID not initialized", sizeof(result.error));

    hidSerial(result.error);

    return result;

  }

  if (!UsbHid.isHostConnected()) {

    strncpy(result.error, "USB host not ready — plug into PC, wait 2s, retry", sizeof(result.error));

    hidSerial(result.error);

    return result;

  }



  if (actionNeedsPayload(action) && (!payload || payload[0] == '\0')) {

    strncpy(result.error, payloadRequiredMessage(action), sizeof(result.error));

    hidSerial(result.error);

    return result;

  }



  if (execute(os, action, payload)) {

    snprintf(_lastMessage, sizeof(_lastMessage), "%s sent (%s)", hidActionName(action), hidOsName(os));

    snprintf(result.error, sizeof(result.error), "OK — %s sent to %s host", hidActionName(action), hidOsName(os));

    result.success = true;

    hidSerial(result.error);

    Log.activity("hid", hidActionName(action), "ok");

    Log.info("hid", result.error);

  } else {

    strncpy(_lastMessage, UsbHid.lastError(), sizeof(_lastMessage) - 1);

    strncpy(result.error, UsbHid.lastError(), sizeof(result.error));

    hidSerial(result.error);

    Log.activity("hid", hidActionName(action), "fail");

    Log.warn("hid", UsbHid.lastError());

  }



  return result;

}



HidLabResult HidLabApp::runAction(HidTargetOs os, const char* actionName, const char* labPin, const char* payload) {
  return runAction(os, hidActionFromString(actionName), labPin, payload);
}

HidLabResult HidLabApp::runScriptContent(const char* labPin, const char* script, HidTargetOs os) {
  HidLabResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!script || script[0] == '\0') {
    strncpy(result.error, "Script is empty", sizeof(result.error) - 1);
    return result;
  }
  if (strlen(script) > HidPayloadStore::kMaxScriptBytes) {
    strncpy(result.error, "Script too large (max 4 KB)", sizeof(result.error) - 1);
    return result;
  }

  HidDuckyEngine engine;
  const HidDuckyResult duck = engine.runScript(script, os);
  result.success = duck.success;
  strncpy(result.error, duck.error, sizeof(result.error) - 1);
  if (duck.success) {
    snprintf(_lastMessage, sizeof(_lastMessage), "Inline script (%u lines)", duck.linesExecuted);
    Log.activity("hid", "ducky_run", "inline");
  }
  return result;
}

HidLabResult HidLabApp::runPayloadScript(const char* labPin, const char* payloadName, HidTargetOs os) {
  HidLabResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!payloadName || payloadName[0] == '\0') {
    strncpy(result.error, "Payload name required", sizeof(result.error) - 1);
    return result;
  }
  if (!HidPayloads.payloadExists(payloadName)) {
    strncpy(result.error, "Payload not found on SD/flash", sizeof(result.error) - 1);
    return result;
  }

  String script;
  if (!HidPayloads.readPayload(payloadName, script)) {
    strncpy(result.error, "Could not read payload", sizeof(result.error) - 1);
    return result;
  }

  HidDuckyEngine engine;
  const HidDuckyResult duck = engine.runScript(script.c_str(), os);
  result.success = duck.success;
  strncpy(result.error, duck.error, sizeof(result.error) - 1);
  if (duck.success) {
    snprintf(_lastMessage, sizeof(_lastMessage), "Script %s (%u lines)", payloadName, duck.linesExecuted);
    Log.activity("hid", "ducky_run", payloadName);
  }
  return result;
}

HidLabResult HidLabApp::armButtonPayload(const char* labPin, const char* payloadName, HidTargetOs os) {
  HidLabResult result;
  if (!checkPin(labPin, result.error, sizeof(result.error))) {
    return result;
  }
  if (!payloadName || payloadName[0] == '\0') {
    strncpy(result.error, "Payload name required", sizeof(result.error) - 1);
    return result;
  }
  if (!HidPayloads.payloadExists(payloadName)) {
    strncpy(result.error, "Payload not found", sizeof(result.error) - 1);
    return result;
  }

  strlcpy(_buttonPayload, payloadName, sizeof(_buttonPayload));
  _buttonOsOverride = static_cast<uint8_t>(os);
  _buttonArmed = true;
  _buttonArmExpiresMs = millis() + 300000U;
  result.success = true;
  snprintf(result.error, sizeof(result.error), "BOOT long-press armed: %s (5 min)", payloadName);
  strncpy(_lastMessage, result.error, sizeof(_lastMessage) - 1);
  Log.activity("hid", "ducky_arm", payloadName);
  return result;
}

void HidLabApp::disarmButtonPayload() {
  _buttonArmed = false;
  _buttonPayload[0] = '\0';
  _buttonArmExpiresMs = 0;
}

bool HidLabApp::tryButtonPayload() {
  if (!_buttonArmed || _buttonPayload[0] == '\0') {
    return false;
  }
  if (millis() >= _buttonArmExpiresMs) {
    disarmButtonPayload();
    return false;
  }
  if (!_ready || !UsbHid.isHostConnected()) {
    return false;
  }

  String script;
  if (!HidPayloads.readPayload(_buttonPayload, script)) {
    disarmButtonPayload();
    return false;
  }

  HidTargetOs os = HidTargetOs::Windows;
  if (_buttonOsOverride < static_cast<uint8_t>(HidTargetOs::Count)) {
    os = static_cast<HidTargetOs>(_buttonOsOverride);
  }

  HidDuckyEngine engine;
  const HidDuckyResult duck = engine.runScript(script.c_str(), os);
  disarmButtonPayload();
  if (duck.success) {
    snprintf(_lastMessage, sizeof(_lastMessage), "BOOT ran %s", _buttonPayload);
    Log.activity("hid", "ducky_button", _buttonPayload);
  }
  return duck.success;
}



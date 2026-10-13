#pragma once

#include "HidOs.h"

#include <Arduino.h>

struct HackCardConfig;

struct HidLabResult {
  bool success = false;
  char error[96] = {0};
};

class HidLabApp {
public:
  bool begin();
  bool isReady() const { return _ready; }

  HidLabResult runAction(HidTargetOs os, HidAction action, const char* labPin, const char* payload);
  HidLabResult runAction(HidTargetOs os, const char* actionName, const char* labPin, const char* payload);
  HidLabResult runPayloadScript(const char* labPin, const char* payloadName, HidTargetOs os);
  HidLabResult runScriptContent(const char* labPin, const char* script, HidTargetOs os);
  HidLabResult armButtonPayload(const char* labPin, const char* payloadName, HidTargetOs os);
  void disarmButtonPayload();
  bool tryButtonPayload();
  HidLabResult status() const;
  String statusToJson(HidTargetOs os) const;
  String armStatusToJson() const;
  const char* lastMessage() const { return _lastMessage; }
  bool isButtonArmed() const { return _buttonArmed; }

  bool setTargetOs(HackCardConfig& config, HidTargetOs os);
  HidTargetOs getTargetOs(const HackCardConfig& config) const;

private:
  bool checkPin(const char* labPin, char* error, size_t errorLen) const;
  bool execute(HidTargetOs os, HidAction action, const char* payload);

  bool _ready = false;
  char _lastMessage[96] = {0};
  bool _buttonArmed = false;
  char _buttonPayload[48] = {0};
  uint8_t _buttonOsOverride = 255;
  uint32_t _buttonArmExpiresMs = 0;
};

extern HidLabApp HidLab;

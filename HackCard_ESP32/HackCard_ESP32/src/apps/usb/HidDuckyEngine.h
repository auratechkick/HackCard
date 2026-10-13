#pragma once

#include "HidOs.h"

#include <Arduino.h>

struct HidDuckyResult {
  bool success = false;
  char error[96] = {0};
  uint16_t linesExecuted = 0;
};

class HidDuckyEngine {
 public:
  HidDuckyResult runScript(const char* script, HidTargetOs os);

 private:
  bool executeLine(const char* line, HidTargetOs& os, uint8_t repeatNext);
  bool runModifierLine(const char* cmd, const char* args, HidTargetOs os);
  bool runActionLine(const char* actionName, HidTargetOs os);
  bool resolveKeyToken(const char* token, uint8_t& outKey) const;
  bool pressTokenKey(const char* token);
  bool pressModifierChord(uint8_t mod1, uint8_t mod2, uint8_t mod3, uint8_t key);

  uint16_t _defaultDelayMs = 0;
  uint16_t _linesExecuted = 0;
};

#pragma once

#ifndef HACKCARD_APP_CONTEXT_H
#define HACKCARD_APP_CONTEXT_H

#include "config/board_config.h"
#include "config/user_config.h"

#include <Arduino.h>

struct HackCardConfig {
  char ownerName[64];
  char title[64];
  char company[64];
  char email[96];
  char phone[32];
  char website[128];
  char linkedin[128];
  char github[128];
  char bio[256];
  char apSsid[32];
  char apPassword[64];
  char staSsid[33];
  char staPassword[64];
  bool staCredentialsSaved;
  char labModePin[16];
  uint8_t apChannel;
  uint8_t ringBrightness;
  uint8_t buzzerVolume;
  uint8_t tuneSuccess;
  uint8_t tuneError;
  uint8_t tuneBoot;
  uint8_t tuneNfc;
  uint8_t bootRingPattern;
  uint8_t bootRingR;
  uint8_t bootRingG;
  uint8_t bootRingB;
  uint8_t wifiLedPattern;
  uint8_t wifiLedR;
  uint8_t wifiLedG;
  uint8_t wifiLedB;
  uint8_t wifiRingPattern;
  uint8_t wifiRingR;
  uint8_t wifiRingG;
  uint8_t wifiRingB;
  bool bootAnimation;
  bool buzzerEnabled;
  bool activityLogging;
  bool firstBootComplete;
  bool captivePortalEnabled;
  uint8_t deviceMode;
  uint8_t hidTargetOs;
};

struct AppContext {
  HackCardConfig config;

  void applyUserConfigDefaults();
};

#endif  // HACKCARD_APP_CONTEXT_H

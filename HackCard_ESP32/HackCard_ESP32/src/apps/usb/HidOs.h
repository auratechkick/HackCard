#pragma once



#include <Arduino.h>



enum class HidTargetOs : uint8_t {

  Windows = 0,

  MacOS = 1,

  Linux = 2,

  Count

};



enum class HidAction : uint8_t {

  Notepad = 0,

  TypeText,

  OpenWebsite,

  LockScreen,

  UnlockScreen,

  Calculator,

  Screenshot,

  Terminal,

  TerminalInfo,

  AppSwitcher,
  CloseWindow,
  VolumeMute,

  VolumeUp,

  VolumeDown,

  MediaPlayPause,

  MediaNext,

  MediaPrevious,

  MouseClick,

  MouseMove,

  KeyPress,

};



const char* hidOsName(HidTargetOs os);

const char* hidActionName(HidAction action);

HidTargetOs hidOsFromString(const char* name);

HidAction hidActionFromString(const char* name);



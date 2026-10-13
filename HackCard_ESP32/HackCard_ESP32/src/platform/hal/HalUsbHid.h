#pragma once



#include "../../apps/usb/HidOs.h"



#include <Arduino.h>



class HalUsbHid {

public:

  bool begin();

  bool isReady() const { return _ready; }

  bool isHostConnected() const;

  bool waitForHost(uint32_t timeoutMs) const;



  bool openNotepad(HidTargetOs os);

  bool openCalculator(HidTargetOs os);

  bool openTerminal(HidTargetOs os);

  bool runTerminalInfo(HidTargetOs os);

  bool typeTextPayload(const char* text);

  bool openWebsite(HidTargetOs os, const char* url);

  bool lockScreen(HidTargetOs os);

  bool unlockScreen(HidTargetOs os, const char* password);

  bool takeScreenshot(HidTargetOs os);

  bool appSwitcher(HidTargetOs os);
  bool closeWindow(HidTargetOs os);
  bool pressNamedKey(const char* keyName);
  bool pressChord(uint8_t mod1, uint8_t mod2, uint8_t key);
  bool runLauncherCommand(HidTargetOs os, const char* command);



  bool volumeMute();

  bool volumeUp();

  bool volumeDown();

  bool mediaPlayPause();

  bool mediaNext();

  bool mediaPrevious();



  bool mouseMove(int8_t dx, int8_t dy);

  bool mouseClick(const char* button);



  const char* lastError() const { return _lastError; }



private:

  bool ensureHostReady();

  bool pressKey(uint8_t key);

  bool pressCombo(uint8_t mod1, uint8_t mod2, uint8_t key);

  bool releaseAllKeys();

  bool openLauncher(HidTargetOs os);

  bool typeText(const char* text);

  bool sendEnter();

  bool consumerPress(uint16_t code);



  bool _ready = false;

  char _lastError[96] = {0};

};



extern HalUsbHid UsbHid;



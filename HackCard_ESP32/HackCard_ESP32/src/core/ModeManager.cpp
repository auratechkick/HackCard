#include "ModeManager.h"

#include "AppContext.h"

extern AppContext gApp;

void ModeManager::apply(DeviceMode mode) {
  DeviceModeManager::apply(gApp, mode);
}

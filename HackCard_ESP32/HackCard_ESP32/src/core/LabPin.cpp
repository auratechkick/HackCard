#include "LabPin.h"

#include "AppContext.h"
#include "config/user_config.h"

#include <cstring>

extern AppContext gApp;

bool labPinProvided(const char* pin) {
  return pin && pin[0] != '\0';
}

bool labPinMatches(const char* pin) {
  if (!labPinProvided(pin)) {
    return false;
  }
  const char* expected = gApp.config.labModePin;
  if (!expected[0]) {
    expected = LAB_MODE_PIN;
  }
  return strcmp(pin, expected) == 0;
}

bool labPinCheck(const char* pin, char* error, size_t errorLen) {
  if (!labPinProvided(pin)) {
    strncpy(error, "Lab PIN required — enter your PIN first", errorLen);
    return false;
  }
  if (!labPinMatches(pin)) {
    strncpy(error, "Invalid lab PIN", errorLen);
    return false;
  }
  return true;
}

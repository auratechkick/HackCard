#include "AppContext.h"

#include "DeviceModeManager.h"
#include "../apps/usb/HidOs.h"

#include <cstring>

void AppContext::applyUserConfigDefaults() {
  std::strncpy(config.ownerName, USER_NAME, sizeof(config.ownerName) - 1);
  std::strncpy(config.title, USER_TITLE, sizeof(config.title) - 1);
  std::strncpy(config.company, USER_COMPANY, sizeof(config.company) - 1);
  std::strncpy(config.email, USER_EMAIL, sizeof(config.email) - 1);
  std::strncpy(config.phone, USER_PHONE, sizeof(config.phone) - 1);
  std::strncpy(config.website, USER_WEBSITE, sizeof(config.website) - 1);
  std::strncpy(config.linkedin, USER_LINKEDIN, sizeof(config.linkedin) - 1);
  std::strncpy(config.github, USER_GITHUB, sizeof(config.github) - 1);
  std::strncpy(config.bio, USER_BIO, sizeof(config.bio) - 1);
  std::strncpy(config.apSsid, AP_SSID, sizeof(config.apSsid) - 1);
  std::strncpy(config.apPassword, AP_PASSWORD, sizeof(config.apPassword) - 1);
  config.staSsid[0] = '\0';
  config.staPassword[0] = '\0';
  config.staCredentialsSaved = false;
  std::strncpy(config.labModePin, LAB_MODE_PIN, sizeof(config.labModePin) - 1);
  config.apChannel = AP_CHANNEL;
  config.ringBrightness = DEFAULT_RING_BRIGHTNESS;
  config.buzzerVolume = DEFAULT_BUZZER_VOLUME;
  config.tuneSuccess = 0;
  config.tuneError = 0;
  config.tuneBoot = 0;
  config.tuneNfc = 0;
  config.bootRingPattern = 0;
  config.bootRingR = 0;
  config.bootRingG = 120;
  config.bootRingB = 255;
  config.wifiLedPattern = 0;
  config.wifiLedR = 0;
  config.wifiLedG = 180;
  config.wifiLedB = 60;
  config.wifiRingPattern = 0;
  config.wifiRingR = 0;
  config.wifiRingG = 40;
  config.wifiRingB = 255;
  config.bootAnimation = BOOT_ANIMATION;
  config.buzzerEnabled = BUZZER_ENABLED;
  config.activityLogging = ACTIVITY_LOGGING;
  config.firstBootComplete = false;
  config.captivePortalEnabled = CAPTIVE_PORTAL_ENABLED;
  config.deviceMode = static_cast<uint8_t>(DeviceMode::Lab);
  config.hidTargetOs = static_cast<uint8_t>(HidTargetOs::Windows);
}

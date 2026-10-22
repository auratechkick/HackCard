/*
 * HackCard RP2350 firmware 0.1.0
 * --------------------------------
 * Serial lab console for the RP2350 HackCard.
 * This MCU has no Wi-Fi and no BLE. The ESP32 Lab Console web UI
 * is not part of this firmware.
 *
 * First-time setup:
 *   1. Run scripts/install-library.ps1  (puts HackCard in Arduino libraries)
 *   2. Open this sketch
 *   3. Tools: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK
 *   4. Upload, then Serial Monitor at 115200, type: help
 *
 * Edit config/user_config.h for the contact card.
 */

#include <HackCardRP2350.h>

#include "src/SerialLab.h"
#include "config/user_config.h"
#include "config/app_registry.h"

void setup() {
  Serial.begin(115200);
  delay(1200);

  RgbRing.begin(DEFAULT_RING_BRIGHTNESS);
  Buzzer.begin(BUZZER_ENABLED);
  Buttons.begin();

#if APP_USB_HID
  UsbHid.begin();
#endif

  if (BOOT_ANIMATION) {
    RgbRing.setPattern(RingPattern::BootAnim);
    Buzzer.playBoot();
  }

#if APP_NFC_READ
  if (!Nfc.begin()) {
    Serial.println("NFC not detected — check I2C1 SDA 6 / SCL 7");
  }
#endif

#if APP_SD
  if (HackSd.begin() && SD_AUTO_CREATE_FOLDERS) {
    HackSd.ensureLabFolders();
    RgbRing.setPattern(RingPattern::FullMode);
  } else {
    RgbRing.setPattern(RingPattern::FlashMode);
  }
#endif

  serialLabBegin();
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  serialLabUpdate();
}

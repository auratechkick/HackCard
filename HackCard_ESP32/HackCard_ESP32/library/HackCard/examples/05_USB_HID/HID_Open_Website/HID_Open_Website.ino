/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID open website
 * FULL FIRMWARE: /hid Open URL
 *
 * WHAT IT DOES:
 * Opens browser and navigates to a URL using keyboard shortcuts.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Edit URL; focus desktop; wait for countdown.
 *
 * LIBRARIES: USB, USBHIDKeyboard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

static const char* kUrl = "https://hardwarehackspace.github.io/hackcard-docs/";
enum class TargetOs { Windows, Mac, Linux };
static const TargetOs kOs = TargetOs::Windows;  // EDIT ME

void openLauncher() {
  delay(400);
  if (kOs == TargetOs::Windows) {
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');
    Keyboard.releaseAll();
  } else if (kOs == TargetOs::Mac) {
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press(' ');
    Keyboard.releaseAll();
  } else {
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_F2);
    Keyboard.releaseAll();
  }
  delay(600);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — HID Open Website (web: /hid open_website)");
  Keyboard.begin();
  USB.begin();
  for (int i = 5; i > 0; i--) {
    Serial.printf("Opening in %d...\n", i);
    delay(1000);
  }
  openLauncher();
  Keyboard.print(kUrl);
  Keyboard.write(KEY_RETURN);
  Serial.println("Done.");
}

void loop() { delay(1000); }

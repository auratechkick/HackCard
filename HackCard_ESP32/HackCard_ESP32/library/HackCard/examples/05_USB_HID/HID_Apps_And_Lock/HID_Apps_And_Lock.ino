/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID apps and lock
 * FULL FIRMWARE: /hid Apps / Lock
 *
 * WHAT IT DOES:
 * Demonstrates Win/Linux shortcut to lock or open Run dialog.
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
 *   4. Select OS in EDIT ME; test on your own PC.
 *
 * LIBRARIES: USB, USBHIDKeyboard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

enum class TargetOs { Windows, Mac, Linux };
static const TargetOs kOs = TargetOs::Windows;  // EDIT ME

void guiKey(char c) {
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press(c);
  Keyboard.releaseAll();
  delay(500);
}

void openRun(const char* cmd) {
  if (kOs == TargetOs::Windows) {
    guiKey('r');
  } else if (kOs == TargetOs::Mac) {
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press(' ');
    Keyboard.releaseAll();
    delay(500);
  } else {
    Keyboard.press(KEY_LEFT_ALT);
    Keyboard.press(KEY_F2);
    Keyboard.releaseAll();
    delay(500);
  }
  Keyboard.print(cmd);
  Keyboard.write(KEY_RETURN);
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — HID Apps & Lock (web: /hid apps)");
  Keyboard.begin();
  USB.begin();
  for (int i = 4; i > 0; i--) {
    Serial.printf("Ready in %d...\n", i);
    delay(1000);
  }
  Serial.println("Commands: n=notepad  c=calc  t=terminal  l=lock  s=screenshot");
}

// loop() - keep calling Hal*.update() here
void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  switch (c) {
    case 'n':
    case 'N':
      Serial.println("Notepad");
      openRun(kOs == TargetOs::Windows ? "notepad" :
              kOs == TargetOs::Mac ? "TextEdit" : "gedit");
      break;
    case 'c':
    case 'C':
      Serial.println("Calculator");
      openRun(kOs == TargetOs::Windows ? "calc" :
              kOs == TargetOs::Mac ? "Calculator" : "gnome-calculator");
      break;
    case 't':
    case 'T':
      Serial.println("Terminal");
      openRun(kOs == TargetOs::Windows ? "cmd" :
              kOs == TargetOs::Mac ? "Terminal" : "gnome-terminal");
      break;
    case 'l':
    case 'L':
      Serial.println("Lock");
      if (kOs == TargetOs::Windows) {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('l');
        Keyboard.releaseAll();
      } else if (kOs == TargetOs::Mac) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('q');
        Keyboard.releaseAll();
      } else {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('l');
        Keyboard.releaseAll();
      }
      break;
    case 's':
    case 'S':
      Serial.println("Screenshot");
      if (kOs == TargetOs::Windows) {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press('s');
        Keyboard.releaseAll();
      } else if (kOs == TargetOs::Mac) {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press('4');
        Keyboard.releaseAll();
      } else {
        Keyboard.print("gnome-screenshot");
        // better: PrtSc
        Keyboard.write(KEY_PRINT_SCREEN);
      }
      break;
    default:
      break;
  }
}

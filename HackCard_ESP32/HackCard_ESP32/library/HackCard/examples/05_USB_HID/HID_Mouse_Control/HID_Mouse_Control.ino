/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID mouse
 * FULL FIRMWARE: /hid Mouse
 *
 * WHAT IT DOES:
 * Moves mouse cursor in a small pattern.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Watch cursor on host after countdown.
 *
 * LIBRARIES: USB, USBHIDMouse
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include "USB.h"
#include "USBHIDMouse.h"

USBHIDMouse Mouse;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — HID Mouse (web: /hid mouse)");
  Mouse.begin();
  USB.begin();
  Serial.println("w/a/s/d move | l/r/m click");
}

// loop() - keep calling Hal*.update() here
void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  switch (c) {
    case 'w': Mouse.move(0, -20); break;
    case 's': Mouse.move(0, 20); break;
    case 'a': Mouse.move(-20, 0); break;
    case 'd': Mouse.move(20, 0); break;
    case 'l': Mouse.click(MOUSE_LEFT); break;
    case 'r': Mouse.click(MOUSE_RIGHT); break;
    case 'm': Mouse.click(MOUSE_MIDDLE); break;
    default: return;
  }
  Serial.printf("mouse '%c'\n", c);
}

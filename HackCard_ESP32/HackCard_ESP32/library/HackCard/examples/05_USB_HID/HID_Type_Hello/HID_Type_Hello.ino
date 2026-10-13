/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID type hello
 * FULL FIRMWARE: /hid Type text
 *
 * WHAT IT DOES:
 * Types a short message into the focused PC window via USB keyboard.
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
 *   4. Focus Notepad before countdown ends.
 *
 * LIBRARIES: USB, USBHIDKeyboard (ESP32 TinyUSB)
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — USB HID Type Hello");

  Keyboard.begin();
  USB.begin();

  Serial.println("Focus a text editor on the PC...");
  for (int i = 5; i > 0; i--) {
    Serial.printf("Typing in %d...\n", i);
    delay(1000);
  }

  Keyboard.print("Hello from HackCard!");
  Keyboard.write(KEY_RETURN);
  Serial.println("Done.");
}

// loop() - keep calling Hal*.update() here
void loop() {
  delay(1000);
}

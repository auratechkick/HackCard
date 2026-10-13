/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID key press
 * FULL FIRMWARE: /hid Key press
 *
 * WHAT IT DOES:
 * Sends individual key down/up codes.
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
 *   4. Edit key constant; focus text field on PC.
 *
 * LIBRARIES: USB, USBHIDKeyboard
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
  Serial.println("HackCard — HID Key Press (web: /hid keys)");
  Keyboard.begin();
  USB.begin();
  Serial.println("t=tab e=enter x=esc b=backspace del=Delete(d)");
  Serial.println("arrows: i=up k=down j=left l=right  space=sp");
}

// loop() - keep calling Hal*.update() here
void loop() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line == "t") Keyboard.write(KEY_TAB);
  else if (line == "e") Keyboard.write(KEY_RETURN);
  else if (line == "x") Keyboard.write(KEY_ESC);
  else if (line == "b") Keyboard.write(KEY_BACKSPACE);
  else if (line == "d") Keyboard.write(KEY_DELETE);
  else if (line == "i") Keyboard.write(KEY_UP_ARROW);
  else if (line == "k") Keyboard.write(KEY_DOWN_ARROW);
  else if (line == "j") Keyboard.write(KEY_LEFT_ARROW);
  else if (line == "l") Keyboard.write(KEY_RIGHT_ARROW);
  else if (line == "sp") Keyboard.write(' ');
  else {
    Serial.println("Unknown");
    return;
  }
  Serial.printf("Sent: %s\n", line.c_str());
}

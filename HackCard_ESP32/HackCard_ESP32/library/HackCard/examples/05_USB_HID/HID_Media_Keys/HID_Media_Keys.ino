/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: USB HID media keys
 * FULL FIRMWARE: /hid Media
 *
 * WHAT IT DOES:
 * Sends volume/play media key reports.
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
 *   4. Focus a media app on host PC.
 *
 * LIBRARIES: USB, USBHIDKeyboard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include "USB.h"
#include "USBHIDConsumerControl.h"

USBHIDConsumerControl Consumer;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — HID Media Keys (web: /hid media)");
  Consumer.begin();
  USB.begin();
  Serial.println("m=mute  u=vol+  d=vol-  p=play/pause  n=next  b=prev");
}

// loop() - keep calling Hal*.update() here
void loop() {
  if (!Serial.available()) return;
  char c = Serial.read();
  while (Serial.available()) Serial.read();
  switch (c) {
    case 'm': Consumer.press(CONSUMER_CONTROL_MUTE); break;
    case 'u': Consumer.press(CONSUMER_CONTROL_VOLUME_INCREMENT); break;
    case 'd': Consumer.press(CONSUMER_CONTROL_VOLUME_DECREMENT); break;
    case 'p': Consumer.press(CONSUMER_CONTROL_PLAY_PAUSE); break;
    case 'n': Consumer.press(CONSUMER_CONTROL_SCAN_NEXT); break;
    case 'b': Consumer.press(CONSUMER_CONTROL_SCAN_PREVIOUS); break;
    default: return;
  }
  delay(50);
  Consumer.release();
  Serial.printf("Sent media key '%c'\n", c);
}

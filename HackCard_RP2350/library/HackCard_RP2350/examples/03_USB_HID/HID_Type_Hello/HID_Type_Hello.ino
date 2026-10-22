/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: USB HID type a line
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Types only into the PC this board is plugged into. Focus your own editor.
 */

#include <HackCardRP2350.h>

static const char* kMessage = "Hello from HackCard RP2350!";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — HID Type Hello");

  UsbHid.begin();
  Serial.println("Focus a text editor on this PC...");
  for (int i = 5; i > 0; i--) {
    Serial.printf("Typing in %d...\n", i);
    delay(1000);
  }

  UsbHid.typeText(kMessage);
  UsbHid.tapEnter();
  Serial.println("Done.");
}

void loop() {
  delay(1000);
}

/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: USB HID mouse nudge
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Moves the pointer on the PC this board is plugged into.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — HID Mouse");
  Serial.println("Short press nudges the pointer right. Long press clicks.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(30);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  RgbRing.update();
  Buttons.update();

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      UsbHid.moveMouse(20, 0);
      Serial.println("Moved +20 x");
      break;
    case ButtonGesture::Long:
      UsbHid.clickMouse();
      Serial.println("Click");
      break;
    default:
      break;
  }
}

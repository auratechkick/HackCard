/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: USB HID media key
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK
 * Serial Monitor: 115200
 * ETHICS: Media keys go to the PC this board is plugged into.
 *
 * Short press = volume up. Long press = mute.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — HID Media Keys");

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
      UsbHid.volumeUp();
      Serial.println("Volume up");
      break;
    case ButtonGesture::Long:
      UsbHid.mute();
      Serial.println("Mute");
      break;
    default:
      break;
  }
}

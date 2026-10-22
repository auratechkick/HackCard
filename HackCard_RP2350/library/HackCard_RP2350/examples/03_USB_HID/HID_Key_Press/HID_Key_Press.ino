/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: USB HID key press
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Sends Enter on the PC this board is plugged into.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — HID Key Press");
  Serial.println("Focus a window, then press the user button to send Enter.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(30);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  RgbRing.update();
  Buttons.update();

  if (Buttons.consumeGesture() != ButtonGesture::Short) {
    return;
  }

  Serial.println("Sending Enter");
  UsbHid.tapEnter();
  Buzzer.begin(true);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(300);
  RgbRing.setPattern(RingPattern::Idle);
}

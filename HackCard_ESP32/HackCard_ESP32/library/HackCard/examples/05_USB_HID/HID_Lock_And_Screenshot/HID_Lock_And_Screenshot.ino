/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Lock the screen or take a screenshot
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Affects only the PC this board is plugged into.
 *
 * Windows: Win+L and Win+Shift+S
 * macOS:   Ctrl+Cmd+Q and Cmd+Shift+3
 * Linux:   Super+L and Print Screen (GNOME-style)
 *
 * Short press = lock. Long press = screenshot. w/m/l selects the OS.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” Lock and screenshot");
  Serial.println("Short = lock, long = screenshot, w/m/l = OS");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
}

void applyOs(HackOs os) {
  Host.setOs(os);
  Serial.printf("OS: %s\n", Host.osName());
}

void loop() {
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') applyOs(HackOs::Windows);
    else if (cmd == 'm' || cmd == 'M') applyOs(HackOs::MacOS);
    else if (cmd == 'l' || cmd == 'L') applyOs(HackOs::Linux);
  }

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      Host.countdown(3, "Lock screen");
      Host.lockScreen();
      break;
    case ButtonGesture::Long:
      Host.countdown(2, "Screenshot");
      Host.screenshot();
      Buzzer.playSuccess();
      break;
    default:
      break;
  }
}


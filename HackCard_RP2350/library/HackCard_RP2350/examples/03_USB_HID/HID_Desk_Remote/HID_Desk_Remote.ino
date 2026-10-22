/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: One-button desk remote
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Media keys, lock, and screenshot stay on this PC.
 *
 * Short press  = mute
 * Long press   = lock screen for the selected OS
 * Double press = screenshot for the selected OS
 * Serial + or - = volume. w/m/l = OS. Ring color shows the OS.
 */

#include <HackCardRP2350.h>

void paintOs() {
  switch (Host.os()) {
    case HackOs::MacOS:
      RgbRing.showColor(210, 210, 220);
      break;
    case HackOs::Linux:
      RgbRing.showColor(230, 110, 16);
      break;
    default:
      RgbRing.showColor(16, 80, 220);
      break;
  }
  Serial.printf("OS: %s\n", Host.osName());
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Desk remote");
  Serial.println("Short mute | long lock | double screenshot | +/- volume");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(60);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  paintOs();
}

void loop() {
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') {
      Host.setOs(HackOs::Windows);
      paintOs();
    } else if (cmd == 'm' || cmd == 'M') {
      Host.setOs(HackOs::MacOS);
      paintOs();
    } else if (cmd == 'l' || cmd == 'L') {
      Host.setOs(HackOs::Linux);
      paintOs();
    } else if (cmd == '+' || cmd == '=') {
      UsbHid.volumeUp();
    } else if (cmd == '-' || cmd == '_') {
      UsbHid.volumeDown();
    }
  }

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      UsbHid.mute();
      Serial.println("Mute");
      Buzzer.playTune(BuzzerTuneId::Pulse);
      break;
    case ButtonGesture::Long:
      Host.countdown(2, "Lock screen");
      Host.lockScreen();
      break;
    case ButtonGesture::Double:
      Host.countdown(2, "Screenshot");
      Host.screenshot();
      break;
    default:
      break;
  }
}

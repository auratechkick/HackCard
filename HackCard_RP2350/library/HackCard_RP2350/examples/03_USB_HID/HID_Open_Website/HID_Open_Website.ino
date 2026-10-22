/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Open a website on the selected OS
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Opens a page on the PC this board is plugged into.
 *
 * Edit the URL and the starting OS. Serial w/m/l changes OS before the button press.
 * Windows uses Win+R, macOS uses Spotlight, Linux uses Alt+F2.
 */

#include <HackCardRP2350.h>

static const char* kUrl = "https://example.com";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — HID Open Website");
  Serial.printf("URL: %s\n", kUrl);
  Serial.println("w/m/l = OS. Short press = open the page.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'W') Host.setOs(HackOs::Windows);
    else if (cmd == 'm' || cmd == 'M') Host.setOs(HackOs::MacOS);
    else if (cmd == 'l' || cmd == 'L') Host.setOs(HackOs::Linux);
    Serial.printf("OS: %s\n", Host.osName());
  }

  if (Buttons.consumeGesture() != ButtonGesture::Short) {
    return;
  }

  Host.countdown(3, "Open website");
  Host.openUrl(kUrl);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(600);
  RgbRing.setPattern(RingPattern::Idle);
}

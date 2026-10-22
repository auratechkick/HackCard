/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Open your meeting link on the selected OS
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Opens a URL on the PC this board is plugged into.
 *
 * Put your Zoom, Meet, or Teams link in kUrl.
 * w/m/l selects the OS. Short press opens it.
 */

#include <HackCardRP2350.h>

static const char* kUrl = "https://meet.example.com/hackcard-lab";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Meeting link");
  Serial.printf("Link: %s\n", kUrl);
  Serial.println("w/m/l = OS. Short press = open.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  Host.setOs(HackOs::Windows);
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

  Host.countdown(3, "Open meeting");
  Host.openUrl(kUrl);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

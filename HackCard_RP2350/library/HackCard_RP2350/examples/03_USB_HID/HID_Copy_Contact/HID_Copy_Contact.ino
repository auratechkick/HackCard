/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Put your contact on the clipboard
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Types and copies on the PC this board is plugged into.
 *
 * Opens the notes app, types the contact, selects it, and copies it.
 * Paste into a form with Ctrl+V (Windows/Linux) or Cmd+V (macOS).
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";
static const char* kEmail = "you@example.com";
static const char* kPhone = "+1 000 000 0000";
static const char* kWeb = "https://example.com";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Copy contact");
  Serial.println("w/m/l = OS. Short press = notes + copy.");

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

  Host.countdown(3, "Copy contact");
  Host.openNotes();
  Host.typeLine(kName);
  Host.typeLine(kEmail);
  Host.typeLine(kPhone);
  Host.typeLine(kWeb);
  delay(200);
  Host.selectAll();
  delay(100);
  Host.copyClipboard();
  Serial.printf("Copied. Paste with %s+V\n", Host.os() == HackOs::MacOS ? "Cmd" : "Ctrl");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

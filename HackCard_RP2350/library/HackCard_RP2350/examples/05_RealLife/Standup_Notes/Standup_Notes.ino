/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Open notes and type a standup
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Opens the notes app and types on the PC this board is plugged into.
 *
 * w/m/l selects Windows Notepad, macOS TextEdit, or Linux gedit.
 * Short press fills Yesterday / Today / Blockers.
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Standup notes");
  Serial.println("w/m/l = OS. Short press = open notes and type the standup.");

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

  Host.countdown(3, "Standup");
  Host.openNotes();
  Host.typeText("Standup — ");
  Host.typeLine(kName);
  Host.typeLine("");
  Host.typeLine("Yesterday:");
  Host.typeLine("- ");
  Host.typeLine("Today:");
  Host.typeLine("- ");
  Host.typeLine("Blockers:");
  Host.typeLine("- ");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Open the notes app and type a meeting template
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Opens an app and types on the PC this board is plugged into.
 *
 * Windows opens Notepad, macOS opens TextEdit, Linux opens gedit.
 * Pick the OS with w/m/l, then short-press to run.
 */

#include <HackCardRP2350.h>

static const char* kMeeting = "Weekly lab sync";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Meeting notes");
  Serial.println("w/m/l = OS. Short press = open notes and type the template.");

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

  Host.countdown(3, "Meeting notes");
  Host.openNotes();
  Host.typeLine(kMeeting);
  Host.typeLine("Attendees:");
  Host.typeLine("- ");
  Host.typeLine("Decisions:");
  Host.typeLine("- ");
  Host.typeLine("Next:");
  Host.typeLine("- ");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
}

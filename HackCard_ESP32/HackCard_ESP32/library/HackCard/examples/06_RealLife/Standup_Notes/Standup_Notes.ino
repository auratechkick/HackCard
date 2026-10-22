/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Open notes and type a standup
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Opens the notes app and types on the PC this board is plugged into.
 *
 * w/m/l selects Windows Notepad, macOS TextEdit, or Linux gedit.
 * Short press fills Yesterday / Today / Blockers.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static const char* kName = "Your Name";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” Standup notes");
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
  Host.typeText("Standup â€” ");
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


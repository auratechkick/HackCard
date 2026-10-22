/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Type an email signature into the focused window
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Types only into the editor you focus on this PC.
 *
 * Open your mail signature box first, then press the button.
 * w/m/l only changes the paste hint after the text is written.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static const char* kName = "Your Name";
static const char* kTitle = "Security Researcher";
static const char* kCompany = "Your Company";
static const char* kEmail = "you@example.com";
static const char* kWeb = "https://example.com";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” Email signature");
  Serial.println("Focus the signature box, then short-press the button.");

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

  Host.countdown(3, "Type signature");
  Host.typeLine(kName);
  Host.typeLine(kTitle);
  Host.typeLine(kCompany);
  Host.typeLine(kEmail);
  Host.typeLine(kWeb);
  Serial.printf("Done. Copy hint: %s\n", Host.os() == HackOs::MacOS ? "Cmd+A, Cmd+C" : "Ctrl+A, Ctrl+C");
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(500);
  RgbRing.setPattern(RingPattern::Idle);
}


/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Type a conference introduction into chat
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Types into the chat box you focus on this PC.
 *
 * Edit the fields. Focus Slack, Teams, or a document, then press the button.
 */

#include <HackCardRP2350.h>

static const char* kName = "Your Name";
static const char* kCompany = "Your Company";
static const char* kLine = "I build small security-lab tools.";
static const char* kWeb = "https://example.com";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Conference intro");
  Serial.println("Focus the chat box, then short-press.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(50);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  Buttons.update();
  if (Buttons.consumeGesture() != ButtonGesture::Short) {
    return;
  }

  Host.countdown(3, "Type intro");
  Host.typeText("Hi, I'm ");
  Host.typeText(kName);
  Host.typeText(" from ");
  Host.typeText(kCompany);
  Host.typeText(". ");
  Host.typeText(kLine);
  Host.typeText(" ");
  Host.typeLine(kWeb);
  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(700);
  RgbRing.setPattern(RingPattern::Idle);
}

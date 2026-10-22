/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Type an away message into the focused chat
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Types into the chat you focus on this PC.
 *
 * Focus Slack, Teams, or a status box. Short press sends the away line
 * and turns the ring amber. Long press sends the "back" line and turns it green.
 */

#include <HackCardRP2350.h>

static const char* kAway = "Stepping away — back in 15 minutes.";
static const char* kBack = "Back at my desk.";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Break note");
  Serial.println("Focus chat. Short = away. Long = back.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(55);
  Buzzer.begin(true);
  RgbRing.showColor(0, 140, 40);
}

void loop() {
  Buttons.update();

  const ButtonGesture gesture = Buttons.consumeGesture();
  if (gesture != ButtonGesture::Short && gesture != ButtonGesture::Long) {
    return;
  }

  const bool away = gesture == ButtonGesture::Short;
  Host.countdown(3, away ? "Away note" : "Back note");
  Host.typeLine(away ? kAway : kBack);
  if (away) {
    RgbRing.showColor(220, 140, 0);
  } else {
    RgbRing.showColor(0, 160, 50);
  }
  Buzzer.playSuccess();
}

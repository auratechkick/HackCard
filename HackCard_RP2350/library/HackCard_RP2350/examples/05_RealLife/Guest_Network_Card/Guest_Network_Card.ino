/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Type your guest network into the focused field
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * ETHICS: Types the network you configure, on the PC this board is plugged into.
 *
 * Edit the SSID and passphrase. Focus the Wi-Fi password box or a note, then press.
 * Short press types the password and Enter. Long press types the network name.
 */

#include <HackCardRP2350.h>

static const char* kSsid = "HackCard-Guest";
static const char* kPass = "change-me";

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Guest network card");
  Serial.println("Focus the password box.");
  Serial.println("Short = type password. Long = type the network name.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  Buttons.update();

  const ButtonGesture gesture = Buttons.consumeGesture();
  if (gesture == ButtonGesture::None) {
    return;
  }

  Host.countdown(3, gesture == ButtonGesture::Long ? "Type network name" : "Type password");
  if (gesture == ButtonGesture::Long) {
    Host.typeLine(kSsid);
  } else if (gesture == ButtonGesture::Short) {
    Host.typeLine(kPass);
  } else {
    return;
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(400);
  RgbRing.setPattern(RingPattern::Idle);
}

/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Desk status light
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * Short press = next status. Long press = sound on/off.
 * Serial: 1 available, 2 busy, 3 away, 4 focus.
 */

#include <HackCardRP2350.h>

enum class DeskStatus : uint8_t { Available, Busy, Away, Focus, Count };

static DeskStatus gStatus = DeskStatus::Available;
static bool gSound = true;

void showStatus() {
  switch (gStatus) {
    case DeskStatus::Available:
      RgbRing.showColor(0, 180, 40);
      Serial.println("Status: AVAILABLE");
      break;
    case DeskStatus::Busy:
      RgbRing.showColor(220, 20, 20);
      Serial.println("Status: BUSY");
      break;
    case DeskStatus::Away:
      RgbRing.showColor(220, 140, 0);
      Serial.println("Status: AWAY");
      break;
    case DeskStatus::Focus:
      RgbRing.showColor(30, 80, 255);
      Serial.println("Status: FOCUS");
      break;
    default:
      break;
  }
  if (gSound) {
    Buzzer.playSuccess();
  }
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Desk Status Light");

  Buttons.begin();
  RgbRing.begin(70);
  Buzzer.begin(true);
  showStatus();
}

void loop() {
  Buttons.update();

  if (Serial.available()) {
    const char cmd = Serial.read();
    if (cmd >= '1' && cmd <= '4') {
      gStatus = static_cast<DeskStatus>(cmd - '1');
      showStatus();
    }
  }

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      gStatus = static_cast<DeskStatus>(
          (static_cast<uint8_t>(gStatus) + 1) % static_cast<uint8_t>(DeskStatus::Count));
      showStatus();
      break;
    case ButtonGesture::Long:
      gSound = !gSound;
      Serial.printf("Sound: %s\n", gSound ? "ON" : "OFF");
      if (gSound) {
        Buzzer.playTune(BuzzerTuneId::Chime);
      }
      break;
    default:
      break;
  }
}

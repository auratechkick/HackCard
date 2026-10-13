/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Desk status light
 * FULL FIRMWARE: Ring as status indicator
 *
 * WHAT IT DOES:
 * Uses ring colors for busy/available/focus states via Serial commands.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Type 1/2/3 on Serial to switch states.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>

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

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — Desk Status Light");
  Serial.println("Short press = next status | Long press = toggle sound");

  Buttons.begin();
  RgbRing.begin(70);
  Buzzer.begin(true);
  showStatus();
}

// loop() - keep calling Hal*.update() here
void loop() {
  Buttons.update();

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      gStatus = static_cast<DeskStatus>(
          (static_cast<uint8_t>(gStatus) + 1) % static_cast<uint8_t>(DeskStatus::Count));
      showStatus();
      break;

    case ButtonGesture::Long:
      gSound = !gSound;
      Serial.printf("Sound: %s\n", gSound ? "ON" : "OFF");
      if (gSound) Buzzer.playTune(BuzzerTuneId::Chime);
      break;

    default:
      break;
  }
}

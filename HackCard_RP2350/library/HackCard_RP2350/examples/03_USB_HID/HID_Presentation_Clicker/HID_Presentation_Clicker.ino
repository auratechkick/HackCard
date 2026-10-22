/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Presentation clicker
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * Focus the slideshow first (PowerPoint, Keynote, or Google Slides).
 * Short press = next slide. Long press = previous. Double press = black screen (B).
 * The ring shows a moving pointer so you can see the click from the podium.
 * Slide keys are the same on Windows, macOS, and Linux.
 */

#include <HackCardRP2350.h>

static uint8_t gSlide = 0;

void showSlide() {
  RgbRing.setPointer(gSlide % 12);
  Serial.printf("Slide mark %u\n", gSlide + 1);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Presentation clicker");
  Serial.println("Focus the slideshow. Short = next, long = back, double = blank.");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(70);
  Buzzer.begin(true);
  RgbRing.setPattern(RingPattern::Idle);
}

void loop() {
  RgbRing.update();
  Buttons.update();

  switch (Buttons.consumeGesture()) {
    case ButtonGesture::Short:
      UsbHid.tapArrowRight();
      gSlide++;
      showSlide();
      Buzzer.playTune(BuzzerTuneId::Pulse);
      break;
    case ButtonGesture::Long:
      UsbHid.tapArrowLeft();
      if (gSlide > 0) {
        gSlide--;
      }
      showSlide();
      break;
    case ButtonGesture::Double:
      UsbHid.tapKey('b');
      Serial.println("Blank screen toggle");
      RgbRing.setPattern(RingPattern::Off);
      break;
    default:
      break;
  }
}

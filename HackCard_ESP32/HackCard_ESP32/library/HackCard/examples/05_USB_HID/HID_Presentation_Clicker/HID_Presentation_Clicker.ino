/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Presentation clicker
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * *
 * Focus the slideshow first (PowerPoint, Keynote, or Google Slides).
 * Short press = next slide. Long press = previous. Double press = black screen (B).
 * The ring shows a moving pointer so you can see the click from the podium.
 * Slide keys are the same on Windows, macOS, and Linux.
 */


 * ETHICS: Use only on PCs, tags, and networks you own or may test.
#include <HackCard.h>
#include <HackCardUsb.h>

static uint8_t gSlide = 0;

void showSlide() {
  RgbRing.setPointer(gSlide % 12);
  Serial.printf("Slide mark %u\n", gSlide + 1);
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” Presentation clicker");
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


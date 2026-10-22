/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: USB HID with Windows / macOS / Linux selection
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Shortcuts run on the PC this board is plugged into.
 *
 * The chip cannot see which OS is attached. You choose it.
 *   w m l     select Windows, macOS, or Linux
 *   button    also cycles the OS (ring color follows)
 *   1 notes   2 calculator   3 terminal   4 website
 *   5 lock    6 screenshot   7 switch app  8 close window
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static const char* kUrl = "https://example.com";

void showOs() {
  switch (Host.os()) {
    case HackOs::MacOS:
      RgbRing.showColor(220, 220, 230);
      break;
    case HackOs::Linux:
      RgbRing.showColor(230, 120, 20);
      break;
    default:
      RgbRing.showColor(20, 90, 220);
      break;
  }
  Serial.printf("OS: %s\n", Host.osName());
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” HID OS menu");
  Serial.println("w/m/l or short press = pick OS");
  Serial.println("1 notes  2 calc  3 terminal  4 website");
  Serial.println("5 lock   6 shot  7 switch    8 close");

  UsbHid.begin();
  Buttons.begin();
  RgbRing.begin(50);
  Buzzer.begin(true);
  showOs();
}

void loop() {
  Buttons.update();
  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    Host.cycleOs();
    Buzzer.playSuccess();
    showOs();
  }

  if (!Serial.available()) {
    return;
  }
  const char cmd = Serial.read();
  while (Serial.available()) {
    Serial.read();
  }

  switch (cmd) {
    case 'w':
    case 'W':
      Host.setOs(HackOs::Windows);
      showOs();
      break;
    case 'm':
    case 'M':
      Host.setOs(HackOs::MacOS);
      showOs();
      break;
    case 'l':
    case 'L':
      Host.setOs(HackOs::Linux);
      showOs();
      break;
    case '1':
      Host.countdown(3, "Open notes");
      Host.openNotes();
      break;
    case '2':
      Host.countdown(3, "Open calculator");
      Host.openCalculator();
      break;
    case '3':
      Host.countdown(3, "Open terminal");
      Host.openTerminal();
      Host.typeLine("echo Hello from HackCard");
      break;
    case '4':
      Host.countdown(3, "Open website");
      Host.openUrl(kUrl);
      break;
    case '5':
      Host.countdown(3, "Lock screen");
      Host.lockScreen();
      break;
    case '6':
      Host.countdown(2, "Screenshot");
      Host.screenshot();
      break;
    case '7':
      Host.appSwitcher();
      Serial.println("App switcher");
      break;
    case '8':
      Host.countdown(2, "Close window");
      Host.closeWindow();
      break;
    default:
      break;
  }
}


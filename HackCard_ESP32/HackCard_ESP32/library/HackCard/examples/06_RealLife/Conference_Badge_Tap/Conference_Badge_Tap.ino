/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Conference badge tap
 * FULL FIRMWARE: Combined NFC + LED demo
 *
 * WHAT IT DOES:
 * Success animation when a badge tag is read.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Tap tag; ring and buzzer confirm.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <SPI.h>
#include <SD.h>

static bool gSdOk = false;

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — Conference Badge Tap");

  RgbRing.begin(55);
  Buzzer.begin(true);
  WifiLed.begin(40);

  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  gSdOk = SD.begin(PIN_SD_CS);
  Serial.printf("SD logging: %s\n", gSdOk ? "ON" : "OFF (optional)");

  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }

  Serial.println("Ready — tap badges on the coil.");
  RgbRing.setPattern(RingPattern::Idle);
  WifiLed.setState(WifiLedState::ApActive);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();

  NfcTagInfo tag;
  if (!Nfc.readTag(tag, 600) || !tag.detected) {
    return;
  }

  uint32_t ms = millis();
  Serial.printf("[%lu] CHECK-IN  UID=%s  type=%s\n",
                ms, tag.uidHex, tag.tagType);

  if (gSdOk) {
    File f = SD.open("/badge_log.txt", FILE_APPEND);
    if (f) {
      f.printf("%lu,%s,%s\n", ms, tag.uidHex, tag.tagType);
      f.close();
    }
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(900);
  RgbRing.setPattern(RingPattern::Idle);
}

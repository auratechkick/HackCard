/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: Activity log demo
 * FULL FIRMWARE: /logs
 *
 * WHAT IT DOES:
 * Appends CSV-style activity lines to Serial (full firmware writes flash/SD).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Trigger events; lines print like dashboard activity log.
 *
 * LIBRARIES: HackCard
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <SPI.h>
#include <SD.h>

static bool gSd = false;

void logEvent(const char* area, const char* action, const char* detail) {
  char line[128];
  snprintf(line, sizeof(line), "%lu,%s,%s,%s",
           (unsigned long)millis(), area, action, detail ? detail : "");
  Serial.println(line);
  if (gSd) {
    File f = SD.open("/activity.log", FILE_APPEND);
    if (f) {
      f.println(line);
      f.close();
    }
  }
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println("HackCard — Activity Log (web: /logs)");
  Buttons.begin();
  RgbRing.begin(40);
  Buzzer.begin(true);
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  gSd = SD.begin(PIN_SD_CS);
  logEvent("system", "boot", gSd ? "sd" : "flash");
  Serial.println("Short BOOT = append sample NFC-style event");
}

// loop() - keep calling Hal*.update() here
void loop() {
  Buttons.update();
  RgbRing.update();
  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    logEvent("nfc", "read", "DEMO-UID");
    Buzzer.playSuccess();
    RgbRing.setPattern(RingPattern::Success);
    delay(500);
    RgbRing.setPattern(RingPattern::Idle);
  }
}

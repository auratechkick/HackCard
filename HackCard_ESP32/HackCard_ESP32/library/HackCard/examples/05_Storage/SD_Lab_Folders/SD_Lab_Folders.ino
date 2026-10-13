/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: SD lab folders
 * FULL FIRMWARE: Settings -> Storage export paths
 *
 * WHAT IT DOES:
 * Creates /nfc, /wifi/exports, /payloads, etc. on SD (matches firmware).
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Run once per new card before Full Mode exports.
 *
 * LIBRARIES: HackCard, SD, SPI
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <SPI.h>
#include <SD.h>

static const char* kFolders[] = {
    "/config",
    "/logs",
    "/nfc",
    "/nfc/dumps",
    "/nfc/exports",
    "/nfc/exports/dumps",
    "/nfc/exports/classic",
    "/nfc/exports/reads",
    "/wifi",
    "/wifi/exports",
    "/ble",
    "/payloads",
};

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard — SD Lab Folders (matches firmware STORAGE.md)");

  RgbRing.begin(40);
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("ERROR: SD mount failed.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  Serial.println("Creating folders:");
  for (const char* path : kFolders) {
    if (!SD.exists(path)) {
      if (SD.mkdir(path)) {
        Serial.printf("  + %s\n", path);
      } else {
        Serial.printf("  ! failed %s\n", path);
      }
    } else {
      Serial.printf("  = %s (exists)\n", path);
    }
  }

  RgbRing.setPattern(RingPattern::FullMode);
  Serial.println("Ready for Full Mode exports from firmware or your own sketches.");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(50);
}

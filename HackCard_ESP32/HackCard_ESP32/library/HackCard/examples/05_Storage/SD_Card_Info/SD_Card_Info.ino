/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: SD card info
 * FULL FIRMWARE: Settings -> Storage (Full Mode)
 *
 * WHAT IT DOES:
 * Mounts microSD over SPI and lists root directory.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. No edits required for first test.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Insert FAT32 card; read size and file list on Serial.
 *
 * LIBRARIES: HackCard, SD, SPI
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 */

#include <HackCard.h>
#include <SPI.h>
#include <SD.h>

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("HackCard — SD Card Info");

  RgbRing.begin(40);
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS)) {
    Serial.println("ERROR: SD mount failed. Check card + FAT32 format.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  uint8_t cardType = SD.cardType();
  Serial.printf("Card type: %u\n", cardType);
  Serial.printf("Size: %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));

  File root = SD.open("/");
  if (!root) {
    Serial.println("Cannot open root");
    return;
  }

  Serial.println("Root listing:");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    Serial.printf("  %s%s  %u bytes\n",
                  entry.name(),
                  entry.isDirectory() ? "/" : "",
                  (unsigned)entry.size());
    entry.close();
  }
  root.close();

  RgbRing.setPattern(RingPattern::Success);
  Serial.println("Done.");
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  delay(20);
}

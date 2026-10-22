/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: SD card info
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 * microSD is on SPI1: CS 25, CLK 26, MOSI 27, MISO 28. Card must be FAT32.
 */

#include <HackCardRP2350.h>

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — SD Card Info");

  RgbRing.begin(40);

  if (!HackSd.begin()) {
    Serial.println("ERROR: SD mount failed. Check the card and FAT32 format.");
    RgbRing.setPattern(RingPattern::Error);
    return;
  }

  Serial.printf("Size: %lu MB\n", static_cast<unsigned long>(HackSd.sizeMb()));
  HackSd.printRootListing(Serial);
  RgbRing.setPattern(RingPattern::Success);
  Serial.println("Done.");
}

void loop() {
  RgbRing.update();
  delay(20);
}

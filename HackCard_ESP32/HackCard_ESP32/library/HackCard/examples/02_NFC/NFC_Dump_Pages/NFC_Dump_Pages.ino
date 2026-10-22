/*
 * HackCard release 1.0.0 example (from RP2350)
 * FEATURE: Dump Type 2 pages to serial and the SD card
  * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *   USB Mode ............. USB-OTG (TinyUSB)
 * * ETHICS: Dump tags you own or are allowed to read.
 *
 * Tap a Type 2 tag. Pages print on serial. If a FAT32 card is inserted,
 * the same text is saved under /nfc/.
 */

#include <HackCard.h>
#include <HackCardUsb.h>

static char gLastUid[64];
static uint32_t gLastMs = 0;

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard ESP32 â€” NFC page dump");

  RgbRing.begin(40);
  Buzzer.begin(true);
  if (HackSd.begin()) {
    HackSd.makeFolder("/nfc");
    Serial.println("SD save: ON");
  } else {
    Serial.println("SD save: OFF");
  }
  if (!Nfc.begin()) {
    Serial.println("ERROR: PN532 not detected");
    RgbRing.setPattern(RingPattern::Error);
    while (true) delay(1000);
  }
  RgbRing.setPattern(RingPattern::Scanning);
  Serial.println("Tap a Type 2 tag.");
}

void loop() {
  RgbRing.update();

  NfcDumpData dump;
  if (!Nfc.dumpType2Tag(dump, 45, 400) || !dump.success) {
    return;
  }
  if (strcmp(gLastUid, dump.tag.uidHex) == 0 && millis() - gLastMs < 2500) {
    return;
  }
  strncpy(gLastUid, dump.tag.uidHex, sizeof(gLastUid) - 1);
  gLastMs = millis();

  Serial.printf("UID %s  pages %u\n", dump.tag.uidHex, dump.pageCount);
  for (uint8_t page = 0; page < dump.pageCount; page++) {
    Serial.printf("  %02u: %02X %02X %02X %02X\n", page, dump.pages[page][0], dump.pages[page][1],
                  dump.pages[page][2], dump.pages[page][3]);
  }

  if (HackSd.isMounted()) {
    char path[80];
    snprintf(path, sizeof(path), "/nfc/dump.txt");
    HackSd.writeText(path, "");
    char line[96];
    snprintf(line, sizeof(line), "UID %s", dump.tag.uidHex);
    HackSd.appendLine(path, line);
    for (uint8_t page = 0; page < dump.pageCount; page++) {
      snprintf(line, sizeof(line), "%02u: %02X %02X %02X %02X", page, dump.pages[page][0], dump.pages[page][1],
               dump.pages[page][2], dump.pages[page][3]);
      HackSd.appendLine(path, line);
    }
    Serial.println("Saved /nfc/dump.txt");
  }

  Buzzer.playSuccess();
  RgbRing.setPattern(RingPattern::Success);
  delay(600);
  RgbRing.setPattern(RingPattern::Scanning);
}


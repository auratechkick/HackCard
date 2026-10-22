/*
 * HackCard RP2350 example 0.1.0
 * FEATURE: Diagnostics self-test
 * Board: Generic RP2350, Flash Size 2MB (no FS), USB Stack Pico SDK, Serial 115200
 *
 * Prints MCU, flash size, PN532, SD, and waits for the user button.
 * BOOT and RUN are hardware-only and are not GPIO-tested.
 */

#include <HackCardRP2350.h>
#include <pico/unique_id.h>

static void printBoardId() {
  pico_unique_board_id_t boardId;
  pico_get_unique_board_id(&boardId);
  Serial.print("Board ID: ");
  for (size_t i = 0; i < sizeof(boardId.id); i++) {
    if (boardId.id[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(boardId.id[i], HEX);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard RP2350 — Diagnostics");
  Serial.printf("Library %s  MCU %s\n", HACKCARD_LIBRARY_VERSION, HACKCARD_BOARD_MCU);

  Serial.printf("CPU: %lu MHz\n", rp2040.f_cpu() / 1000000UL);
  Serial.printf("Heap: %u bytes\n", rp2040.getFreeHeap());
  Serial.printf("Flash: %lu KB\n", static_cast<unsigned long>(PICO_FLASH_SIZE_BYTES / 1024));
  printBoardId();

  RgbRing.begin(40);
  Buzzer.begin(true);
  Buttons.begin();
  Buzzer.playBoot();

  if (Nfc.begin()) {
    Serial.printf("NFC: PASS  %s\n", Nfc.getChipInfo());
  } else {
    Serial.println("NFC: FAIL  PN532 not on I2C1");
  }

  if (HackSd.begin()) {
    Serial.printf("SD:  PASS  %lu MB\n", static_cast<unsigned long>(HackSd.sizeMb()));
    RgbRing.setPattern(RingPattern::FullMode);
  } else {
    Serial.println("SD:  SKIP  no card (optional)");
    RgbRing.setPattern(RingPattern::FlashMode);
  }

  Serial.println("Press the user button within 15 seconds...");
}

void loop() {
  RgbRing.update();
  Buttons.update();

  static bool gDone = false;
  static uint32_t gStart = 0;
  if (gStart == 0) {
    gStart = millis();
  }
  if (gDone) {
    return;
  }

  if (Buttons.consumeGesture() == ButtonGesture::Short) {
    Serial.println("Button: PASS");
    Buzzer.playSuccess();
    RgbRing.setPattern(RingPattern::Success);
    gDone = true;
    return;
  }

  if (millis() - gStart > 15000) {
    Serial.println("Button: FAIL (no press)");
    Buzzer.playError();
    RgbRing.setPattern(RingPattern::Error);
    gDone = true;
  }
}

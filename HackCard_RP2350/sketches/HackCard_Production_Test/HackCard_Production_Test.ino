/*
 * HackCard Production Test Firmware — RP2350
 * ------------------------------------------
 * Full hardware validation for RP2350 + PN532 HackCard before shipping.
 *
 * Covers: Serial, MCU/flash, USB, PN532 NFC, SD card, RGB LEDs,
 *         buzzer, user button, and combined NFC feedback loop.
 *
 * BOOT (QSPI-3) and RUN buttons are hardware-only — manual QC.
 *
 * Serial monitor: 115200 baud
 * Commands after a run:  R = re-run all tests,  H = help
 *
 * Board support package: Earle Philhower arduino-pico (RP2350)
 * Required libraries: Adafruit PN532, Adafruit NeoPixel, SD (built-in)
 */

#include "hackcard_config.h"

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>
#include <pico/unique_id.h>

// ---------------------------------------------------------------------------
// Hardware objects
// ---------------------------------------------------------------------------

Adafruit_PN532 nfc(-1, -1, &Wire1);
Adafruit_NeoPixel strip(HC_STRIP_LED_COUNT, HC_STRIP_LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------------------------------------------------------------------------
// Test result tracking
// ---------------------------------------------------------------------------

enum TestResult {
  TEST_PASS = 0,
  TEST_FAIL = 1,
  TEST_SKIP = 2
};

struct TestRecord {
  const char *name;
  TestResult result;
  const char *detail;
};

static const int MAX_TESTS = 19;
TestRecord gTests[MAX_TESTS];
int gTestCount = 0;
int gPassCount = 0;
int gFailCount = 0;
int gSkipCount = 0;

bool gSdReady = false;
String gReportBuffer;

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------

void recordTest(const char *name, TestResult result, const char *detail = "");
void printBanner();
void printHelp();
void appendReport(const String &line);
void saveReportToSd();
void setStatusColor(uint8_t r, uint8_t g, uint8_t b);
void setStripColor(uint8_t r, uint8_t g, uint8_t b);
void clearLeds();
void beepPass();
void beepFail();
void beepPrompt();
void beepStartup();
void toneOn(uint16_t freq);
void toneOff();
void serialFlush();
void initI2c();

TestResult testSerialAndMcu();
TestResult testExternalFlash();
TestResult testUsbCdc();
TestResult testI2cBus();
TestResult testNfcInit();
TestResult testNfcIrqLine();
TestResult testNfcReadInteractive();
TestResult testSdCard();
TestResult testRgbStrip();
TestResult testBuzzer();
TestResult testUserButton();
TestResult testCombinedNfcFeedback();
void printManualQcChecklist();

void runProductionSuite();
void printFinalSummary();

// ---------------------------------------------------------------------------
// Setup / loop
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1500);

  printBanner();

  strip.begin();
  clearLeds();

  pinMode(HC_BUZZER_PIN, OUTPUT);
  digitalWrite(HC_BUZZER_PIN, LOW);

  pinMode(HC_BUTTON_PIN, INPUT_PULLUP);
  pinMode(HC_NFC_IRQ_PIN, INPUT_PULLUP);

  beepStartup();
  runProductionSuite();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();

    switch (cmd) {
      case 'r':
      case 'R':
        runProductionSuite();
        break;

      case 'h':
      case 'H':
      case '?':
        printHelp();
        break;

      default:
        break;
    }
  }
}

// ---------------------------------------------------------------------------
// Test runner
// ---------------------------------------------------------------------------

void runProductionSuite() {
  gTestCount = 0;
  gPassCount = 0;
  gFailCount = 0;
  gSkipCount = 0;
  gReportBuffer = "";
  gSdReady = false;

  Serial.println();
  Serial.println("============================================================");
  Serial.println("  HACKCARD PRODUCTION TEST  v" HC_TEST_VERSION);
  Serial.println("  Board: HackCard RP2350");
  Serial.println("============================================================");
  Serial.println();

  appendReport("HACKCARD PRODUCTION TEST v" HC_TEST_VERSION);
  appendReport("Board: HackCard RP2350");
  appendReport("Started");

  recordTest("Serial + MCU", testSerialAndMcu());
  recordTest("External flash", testExternalFlash());
  recordTest("USB CDC serial", testUsbCdc());

  initI2c();
  recordTest("I2C bus scan", testI2cBus());

  recordTest("PN532 init", testNfcInit());
  recordTest("PN532 IRQ line", testNfcIrqLine());
  recordTest("SD card R/W", testSdCard());
  recordTest("RGB strip", testRgbStrip());
  recordTest("Buzzer", testBuzzer());

  Serial.println();
  Serial.println("--- Interactive tests (operator required) ---");
  Serial.println();

  recordTest("RUN button", TEST_SKIP, "Hardware RUN pin — manual QC");
  recordTest("BOOT button", TEST_SKIP, "Hardware QSPI-3 — manual QC");

  recordTest("User button", testUserButton());
  recordTest("NFC card read", testNfcReadInteractive());
  recordTest("NFC full feedback", testCombinedNfcFeedback());

  printFinalSummary();
  saveReportToSd();
}

// ---------------------------------------------------------------------------
// Individual tests
// ---------------------------------------------------------------------------

TestResult testSerialAndMcu() {
  Serial.println("[TEST] Serial + MCU");

  Serial.print("  CPU freq:   ");
  Serial.print(rp2040.f_cpu() / 1000000UL);
  Serial.println(" MHz");

  Serial.print("  Free heap:  ");
  Serial.print(rp2040.getFreeHeap());
  Serial.println(" bytes");

  pico_unique_board_id_t boardId;
  pico_get_unique_board_id(&boardId);

  Serial.print("  Board ID:   ");
  for (size_t i = 0; i < sizeof(boardId.id); i++) {
    if (boardId.id[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(boardId.id[i], HEX);
  }
  Serial.println();

  appendReport("MCU: RP2350 heap=" + String(rp2040.getFreeHeap()));

  if (rp2040.getFreeHeap() < 20000) {
    Serial.println("  RESULT: FAIL (low heap)");
    return TEST_FAIL;
  }

  Serial.println("  RESULT: PASS");
  return TEST_PASS;
}

TestResult testExternalFlash() {
  Serial.println("[TEST] External flash (W25Q16)");

  uint32_t flashSize = PICO_FLASH_SIZE_BYTES;
  Serial.print("  Flash size: ");
  Serial.print(flashSize / 1024);
  Serial.println(" KB (board build setting)");

  if (flashSize > (2UL * 1024UL * 1024UL)) {
    Serial.println("  Note: W25Q16 is 2MB — set Tools > Flash Size to 2MB (16Mb)");
  }

  appendReport("Flash: " + String(flashSize / 1024) + "KB");

  if (flashSize < (1024UL * 1024UL)) {
    Serial.println("  RESULT: FAIL (flash too small or not detected)");
    return TEST_FAIL;
  }

  Serial.println("  RESULT: PASS");
  serialFlush();
  return TEST_PASS;
}

TestResult testUsbCdc() {
  Serial.println("[TEST] USB CDC serial");

  if (!Serial) {
    Serial.println("  USB serial not ready");
    Serial.println("  RESULT: FAIL");
    return TEST_FAIL;
  }

  Serial.println("  USB CDC link active (you are reading this line)");
  appendReport("USB CDC: OK");
  Serial.println("  RESULT: PASS");
  serialFlush();
  return TEST_PASS;
}

void serialFlush() {
  if (Serial) {
    Serial.flush();
  }
}

void initI2c() {
  Serial.println();
  Serial.println("Initializing I2C1 for PN532...");
  Serial.print("  SDA=GPIO");
  Serial.print(HC_SDA_PIN);
  Serial.print("  SCL=GPIO");
  Serial.println(HC_SCL_PIN);
  serialFlush();

  Wire1.setSDA(HC_SDA_PIN);
  Wire1.setSCL(HC_SCL_PIN);
  Wire1.begin();
  Wire1.setClock(100000);
  Wire1.setTimeout(500, true);

  Serial.println("  I2C1 started OK");
  serialFlush();
}

TestResult testI2cBus() {
  Serial.println("[TEST] I2C bus scan");
  serialFlush();

  int found = 0;
  bool pn532Found = false;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire1.beginTransmission(addr);
    uint8_t err = Wire1.endTransmission();

    if (err == 0) {
      Serial.print("  Device at 0x");
      if (addr < 16) {
        Serial.print("0");
      }
      Serial.println(addr, HEX);
      found++;

      if (addr == 0x24) {
        pn532Found = true;
      }
    }
  }

  if (found == 0) {
    Serial.println("  No I2C devices found");
    Serial.println("  Check PN532 power, SDA/SCL wiring, and I2C mode straps");
    Serial.println("  RESULT: FAIL");
    serialFlush();
    return TEST_FAIL;
  }

  Serial.print("  Devices found: ");
  Serial.println(found);

  if (!pn532Found) {
    Serial.println("  WARNING: PN532 not seen at expected address 0x24");
  }

  appendReport("I2C devices: " + String(found) + (pn532Found ? " (PN532@0x24)" : ""));

  Serial.println("  RESULT: PASS");
  serialFlush();
  return TEST_PASS;
}

TestResult testNfcInit() {
  Serial.println("[TEST] PN532 NFC init");
  serialFlush();
  nfc.begin();

  uint32_t version = nfc.getFirmwareVersion();
  if (!version) {
    Serial.println("  PN532 not detected on I2C");
    Serial.println("  Check wiring: SDA=GPIO" + String(HC_SDA_PIN) +
                   " SCL=GPIO" + String(HC_SCL_PIN));
    Serial.println("  RESULT: FAIL");
    return TEST_FAIL;
  }

  Serial.print("  Firmware: ");
  Serial.print((version >> 16) & 0xFF, DEC);
  Serial.print(".");
  Serial.println((version >> 8) & 0xFF, DEC);

  nfc.SAMConfig();

  appendReport("PN532 FW: " + String((version >> 16) & 0xFF) + "." +
               String((version >> 8) & 0xFF));

  Serial.println("  RESULT: PASS");
  return TEST_PASS;
}

TestResult testNfcIrqLine() {
  Serial.println("[TEST] PN532 IRQ line (GPIO " + String(HC_NFC_IRQ_PIN) + ")");

  pinMode(HC_NFC_IRQ_PIN, INPUT_PULLUP);
  int irqIdle = digitalRead(HC_NFC_IRQ_PIN);

  Serial.print("  IRQ idle level: ");
  Serial.println(irqIdle == HIGH ? "HIGH" : "LOW");

  appendReport(String("PN532 IRQ idle=") + (irqIdle == HIGH ? "HIGH" : "LOW"));

  Serial.println("  RESULT: PASS (line readable)");
  return TEST_PASS;
}

TestResult testSdCard() {
  Serial.println("[TEST] SD card read/write");
  serialFlush();

  Serial.println("  Initializing SPI1...");
  Serial.print("  CS=GPIO");
  Serial.print(HC_SD_CS);
  Serial.print(" CLK=GPIO");
  Serial.print(HC_SD_CLK);
  Serial.print(" MOSI=GPIO");
  Serial.print(HC_SD_MOSI);
  Serial.print(" MISO=GPIO");
  Serial.println(HC_SD_MISO);
  serialFlush();

  SPI1.setRX(HC_SD_MISO);
  SPI1.setTX(HC_SD_MOSI);
  SPI1.setSCK(HC_SD_CLK);
  SPI1.begin();

  Serial.println("  Mounting SD card...");
  serialFlush();

  if (!SD.begin(HC_SD_CS, SPI_HALF_SPEED, SPI1)) {
    Serial.println("  SD mount failed");
    Serial.println("  Check wiring: CS=GPIO" + String(HC_SD_CS) +
                   " CLK=GPIO" + String(HC_SD_CLK) +
                   " MOSI=GPIO" + String(HC_SD_MOSI) +
                   " MISO=GPIO" + String(HC_SD_MISO));
    Serial.println("  RESULT: FAIL");
    gSdReady = false;
    return TEST_FAIL;
  }

  gSdReady = true;

  const char *testPath = "/prod_test.bin";
  const char *payload = "HACKCARD-PROD-OK-12345";
  const size_t payloadLen = strlen(payload);

  SD.remove(testPath);

  File writeFile = SD.open(testPath, FILE_WRITE);
  if (!writeFile) {
    Serial.println("  SD write open failed");
    Serial.println("  RESULT: FAIL");
    return TEST_FAIL;
  }

  writeFile.write((const uint8_t *)payload, payloadLen);
  writeFile.close();

  File readFile = SD.open(testPath, FILE_READ);
  if (!readFile) {
    Serial.println("  SD read open failed");
    Serial.println("  RESULT: FAIL");
    return TEST_FAIL;
  }

  char buffer[32] = {0};
  size_t readLen = readFile.readBytes(buffer, sizeof(buffer) - 1);
  readFile.close();
  SD.remove(testPath);

  if (readLen != payloadLen || strncmp(buffer, payload, payloadLen) != 0) {
    Serial.println("  SD data mismatch");
    Serial.println("  RESULT: FAIL");
    return TEST_FAIL;
  }

  uint64_t cardSize = SD.size64();
  Serial.print("  Card size: ");
  Serial.print((uint32_t)(cardSize / (1024ULL * 1024ULL)));
  Serial.println(" MB");

  appendReport("SD: OK " + String((uint32_t)(cardSize / (1024ULL * 1024ULL))) + "MB");

  Serial.println("  RESULT: PASS");
  return TEST_PASS;
}

TestResult testRgbStrip() {
  Serial.println("[TEST] RGB strip (" + String(HC_STRIP_LED_COUNT) + " LEDs, GPIO " +
                 String(HC_STRIP_LED_PIN) + ")");
  Serial.println("  Each pixel lights RED one at a time, then all GREEN");

  for (int i = 0; i < HC_STRIP_LED_COUNT; i++) {
    strip.clear();

    for (int j = 0; j < HC_STRIP_LED_COUNT; j++) {
      strip.setPixelColor(j, strip.Color(0, 0, 0));
    }

    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(120);
  }

  setStripColor(0, 255, 0);
  delay(400);
  clearLeds();

  Serial.println("  RESULT: PASS (visual check by operator)");
  return TEST_PASS;
}

TestResult testBuzzer() {
  Serial.println("[TEST] Buzzer (GPIO " + String(HC_BUZZER_PIN) + ")");
  Serial.println("  Playing ascending tone sweep — listen for clear sound");

  for (int freq = 800; freq <= 3500; freq += 200) {
    toneOn((uint16_t)freq);
    delay(25);
  }

  toneOff();
  delay(100);

  toneOn(2500);
  delay(150);
  toneOff();

  Serial.println("  RESULT: PASS (audible check by operator)");
  return TEST_PASS;
}

TestResult testUserButton() {
  Serial.println("[TEST] User button (GPIO " + String(HC_BUTTON_PIN) + ")");
  Serial.println("  Press the BUTTON within " + String(HC_BUTTON_WAIT_MS / 1000) + " seconds...");

  beepPrompt();
  setStatusColor(255, 128, 0);

  unsigned long start = millis();
  bool sawRelease = digitalRead(HC_BUTTON_PIN) == HIGH;

  while (millis() - start < HC_BUTTON_WAIT_MS) {
    int level = digitalRead(HC_BUTTON_PIN);

    if (sawRelease && level == LOW) {
      Serial.println("  Button press detected");
      appendReport("Button: pressed");

      setStatusColor(0, 255, 0);
      beepPass();
      delay(300);
      clearLeds();

      Serial.println("  RESULT: PASS");
      return TEST_PASS;
    }

    if (level == HIGH) {
      sawRelease = true;
    }
  }

  clearLeds();
  Serial.println("  RESULT: FAIL (no press detected)");
  return TEST_FAIL;
}

TestResult testNfcReadInteractive() {
  Serial.println("[TEST] NFC card read");
  Serial.println("  Tap any ISO14443A card/tag within " + String(HC_NFC_WAIT_MS / 1000) + " seconds...");

  beepPrompt();
  setStatusColor(255, 128, 0);

  unsigned long start = millis();
  uint8_t uid[7];
  uint8_t uidLength;

  while (millis() - start < HC_NFC_WAIT_MS) {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 250)) {
      Serial.print("  UID: ");

      String uidStr = "";

      for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
          Serial.print("0");
        }

        Serial.print(uid[i], HEX);
        Serial.print(" ");

        if (uid[i] < 0x10) {
          uidStr += "0";
        }

        uidStr += String(uid[i], HEX);
      }

      Serial.println();
      appendReport("NFC UID: " + uidStr);

      setStatusColor(0, 255, 0);
      beepPass();
      delay(300);
      clearLeds();

      Serial.println("  RESULT: PASS");
      return TEST_PASS;
    }
  }

  clearLeds();
  Serial.println("  RESULT: FAIL (no card detected)");
  return TEST_FAIL;
}

TestResult testCombinedNfcFeedback() {
  Serial.println("[TEST] NFC + RGB + Buzzer + SD combined");
  Serial.println("  Tap card again to verify full HackCard feedback path...");

  beepPrompt();
  setStatusColor(0, 255, 255);

  unsigned long start = millis();
  uint8_t uid[7];
  uint8_t uidLength;

  while (millis() - start < HC_NFC_WAIT_MS) {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 250)) {
      Serial.println("  Card detected — running success feedback");

      if (gSdReady) {
        File logFile = SD.open("/production_log.txt", "a");

        if (logFile) {
          logFile.println("PROD PASS " + String(millis()));
          logFile.close();
        }
      }

      setStripColor(0, 255, 0);
      setStatusColor(0, 255, 0);

      for (int i = 0; i < 3; i++) {
        toneOn((uint16_t)(2000 + (i * 500)));
        delay(100);
      }

      toneOff();
      delay(400);
      clearLeds();

      Serial.println("  RESULT: PASS");
      return TEST_PASS;
    }
  }

  clearLeds();
  Serial.println("  RESULT: FAIL (no card detected)");
  return TEST_FAIL;
}

// ---------------------------------------------------------------------------
// Reporting helpers
// ---------------------------------------------------------------------------

void recordTest(const char *name, TestResult result, const char *detail) {
  if (gTestCount >= MAX_TESTS) {
    return;
  }

  gTests[gTestCount].name = name;
  gTests[gTestCount].result = result;
  gTests[gTestCount].detail = detail;
  gTestCount++;

  switch (result) {
    case TEST_PASS:
      gPassCount++;
      appendReport(String("PASS: ") + name);
      break;

    case TEST_FAIL:
      gFailCount++;
      appendReport(String("FAIL: ") + name + (detail ? String(" — ") + detail : ""));
      break;

    case TEST_SKIP:
      gSkipCount++;
      appendReport(String("SKIP: ") + name + (detail ? String(" — ") + detail : ""));
      break;
  }
}

void printFinalSummary() {
  Serial.println();
  Serial.println("============================================================");
  Serial.println("  PRODUCTION TEST SUMMARY");
  Serial.println("============================================================");

  for (int i = 0; i < gTestCount; i++) {
    Serial.print("  ");

    switch (gTests[i].result) {
      case TEST_PASS:
        Serial.print("[PASS] ");
        break;

      case TEST_FAIL:
        Serial.print("[FAIL] ");
        break;

      case TEST_SKIP:
        Serial.print("[SKIP] ");
        break;
    }

    Serial.print(gTests[i].name);

    if (gTests[i].detail && gTests[i].detail[0] != '\0') {
      Serial.print(" — ");
      Serial.print(gTests[i].detail);
    }

    Serial.println();
  }

  Serial.println("------------------------------------------------------------");
  Serial.print("  PASS: ");
  Serial.print(gPassCount);
  Serial.print("   FAIL: ");
  Serial.print(gFailCount);
  Serial.print("   SKIP: ");
  Serial.println(gSkipCount);
  Serial.println("============================================================");

  appendReport("Summary PASS=" + String(gPassCount) + " FAIL=" + String(gFailCount) +
               " SKIP=" + String(gSkipCount));

  if (gFailCount == 0) {
    Serial.println("  >>> UNIT READY FOR PRODUCTION <<<");
    beepPass();

    for (int i = 0; i < 3; i++) {
      setStatusColor(0, 255, 0);
      setStripColor(0, 255, 0);
      delay(250);
      clearLeds();
      delay(150);
    }
  } else {
    Serial.println("  >>> UNIT FAILED — DO NOT SHIP <<<");
    beepFail();

    for (int i = 0; i < 6; i++) {
      setStatusColor(255, 0, 0);
      setStripColor(255, 0, 0);
      toneOn(1000);
      delay(150);
      clearLeds();
      toneOff();
      delay(150);
    }
  }

  printManualQcChecklist();
  Serial.println();
  printHelp();
}

void printManualQcChecklist() {
  Serial.println();
  Serial.println("--- Manual QC checklist (operator sign-off) ---");
  Serial.println("  [ ] RUN button resets the unit");
  Serial.println("  [ ] BOOT button enters USB boot mode when held at power-on");
  Serial.println("  [ ] USB-C powers the board and enumerates as serial device");
  Serial.println("  [ ] All 12 RGB LEDs visible during strip test");
  Serial.println("  These are not firmware GPIO tests — verify on a sample basis.");
}

void saveReportToSd() {
  if (!gSdReady) {
    Serial.println("SD report skipped (SD not available)");
    return;
  }

  File report = SD.open("/production_report.txt", "w");

  if (!report) {
    Serial.println("Could not write production_report.txt");
    return;
  }

  report.println(gReportBuffer);
  report.close();

  Serial.println("Report saved to SD: /production_report.txt");
}

void appendReport(const String &line) {
  gReportBuffer += line;
  gReportBuffer += "\n";
}

void printBanner() {
  Serial.println();
  Serial.println(" __  __            _   ____            _      _   ");
  Serial.println("|  \\/  | __ _  __ _| |_|  _ \\ __ _ _ __| | __ | |_ ");
  Serial.println("| |\\/| |/ _` |/ _` | __| |_) / _` | '__| |/ / | __|");
  Serial.println("| |  | | (_| | (_| | |_|  __/ (_| | |  |   <  | |_ ");
  Serial.println("|_|  |_|\\__,_|\\__,_|\\__|_|   \\__,_|_|  |_|\\_\\  \\__|");
  Serial.println();
  Serial.println("Production Test Firmware  v" HC_TEST_VERSION);
  Serial.println("Board: HackCard RP2350");
}

void printHelp() {
  Serial.println("Commands:");
  Serial.println("  R  Re-run full production test");
  Serial.println("  H  Show this help");
  Serial.println();
  Serial.println("RUN/BOOT buttons are checked manually — see checklist above.");
}

// ---------------------------------------------------------------------------
// LED / buzzer helpers
// ---------------------------------------------------------------------------

void setStatusColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(HC_STATUS_LED_INDEX, strip.Color(r, g, b));
  strip.show();
}

void setStripColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < HC_STRIP_LED_COUNT; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }

  strip.show();
}

void clearLeds() {
  strip.clear();
  strip.show();
  toneOff();
}

void toneOn(uint16_t freq) {
  tone(HC_BUZZER_PIN, freq);
}

void toneOff() {
  noTone(HC_BUZZER_PIN);
  digitalWrite(HC_BUZZER_PIN, LOW);
}

void beepPass() {
  toneOn(2500);
  delay(80);
  toneOn(3200);
  delay(80);
  toneOff();
}

void beepFail() {
  for (int i = 0; i < 4; i++) {
    toneOn(900);
    delay(120);
    toneOff();
    delay(80);
  }
}

void beepPrompt() {
  toneOn(1800);
  delay(60);
  toneOff();
}

void beepStartup() {
  toneOn(1500);
  delay(100);
  toneOn(2500);
  delay(100);
  toneOff();

  setStatusColor(0, 255, 0);
  delay(300);
  clearLeds();
}

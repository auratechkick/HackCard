/*
 * HackCard release 1.0.0 example
 * -------------------------------------------------------------------------
 * FEATURE: EAPOL capture counter
 * FULL FIRMWARE: /wifi Capture
 *
 * WHAT IT DOES:
 * Counts EAPOL frames on a fixed channel; optional SD log line.
 *
 * BEFORE UPLOAD (Arduino IDE -> Tools):
 *   Board ................ ESP32S3 Dev Module
 *   USB CDC On Boot ...... Enabled
 *
 * HOW TO RUN:
 *   1. File -> Examples -> HackCard -> (this sketch's folder)
 *   2. Edit constants marked EDIT ME at top of file.
 *   3. Upload and open Serial Monitor @ 115200 baud
 *   4. Type PIN; wait for session end; check Serial count.
 *
 * LIBRARIES: HackCard, WiFi, esp_wifi, SD optional
 * INDEX: library/HackCard/EXAMPLES.md | docs/CUSTOM_DEVELOPMENT.md
 * ETHICS: Authorized testing only - networks/tags/PCs you own or have permission to test.
 */

#include <HackCard.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <SPI.h>
#include <SD.h>

// ── EDIT ME ────────────────────────────────────────────
static const char* kLabPin = "1234";
static const uint8_t kChannel = 6;
static const uint32_t kDurationMs = 120000;
// ──────────────────────────────────────────────────────

static volatile bool gActive = false;
static volatile uint32_t gEapol = 0;
static volatile uint32_t gMgmt = 0;

static bool frameIsEapol(const uint8_t* payload, uint16_t len) {
  if (!payload || len < 34) {
    return false;
  }
  const uint8_t frameType = (payload[0] >> 2) & 0x03;
  if (frameType != 2) {
    return false;
  }
  for (uint16_t i = 30; i + 2 < len; i++) {
    if (payload[i] == 0x88 && payload[i + 1] == 0x8E) {
      return true;
    }
  }
  return false;
}

void IRAM_ATTR onSniff(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (!gActive || !buf) {
    return;
  }
  const wifi_promiscuous_pkt_t* pkt = static_cast<wifi_promiscuous_pkt_t*>(buf);
  const uint16_t len = pkt->rx_ctrl.sig_len;
  if (len == 0) {
    return;
  }
  if (type == WIFI_PKT_MGMT) {
    gMgmt++;
    return;
  }
  if (type == WIFI_PKT_DATA && frameIsEapol(pkt->payload, len)) {
    gEapol++;
  }
}

static bool mountSd() {
  SPI.begin(PIN_SD_CLK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  if (!SD.begin(PIN_SD_CS)) {
    return false;
  }
  if (!SD.exists("/wifi")) {
    SD.mkdir("/wifi");
  }
  if (!SD.exists("/wifi/exports")) {
    SD.mkdir("/wifi/exports");
  }
  return true;
}

static void logSummaryToSd() {
  if (!mountSd()) {
    Serial.println("SD not present — summary Serial only.");
    return;
  }
  File f = SD.open("/wifi/exports/eapol_example.log", FILE_APPEND);
  if (!f) {
    Serial.println("Could not write SD log.");
    return;
  }
  f.printf("%lu,eapol=%lu,mgmt=%lu,ch=%u\n", millis(), (unsigned long)gEapol,
           (unsigned long)gMgmt, kChannel);
  f.close();
  Serial.println("Summary appended: /wifi/exports/eapol_example.log");
}

// setup() - board init, Serial, peripherals
void setup() {
  Serial.begin(115200);
  delay(800);
  Serial.println();
  Serial.println("HackCard — EAPOL Capture (web: /wifi capture)");
  Serial.println("Authorized lab only.");
  RgbRing.begin(45);
  WifiLed.begin(70);
  Buzzer.begin(true);
  Serial.printf("Channel %u, max %lu s. PIN (%s) to start...\n",
                kChannel, (unsigned long)(kDurationMs / 1000), kLabPin);
}

// loop() - keep calling Hal*.update() here
void loop() {
  RgbRing.update();
  WifiLed.update();

  static bool sessionDone = false;
  if (sessionDone || !Serial.available()) {
    return;
  }

  String pin = Serial.readStringUntil('\n');
  pin.trim();
  if (pin != kLabPin) {
    Serial.println("PIN rejected.");
    Buzzer.playError();
    return;
  }

  WiFi.mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_channel(kChannel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&onSniff);
  gEapol = 0;
  gMgmt = 0;
  gActive = true;

  Serial.println("Capturing EAPOL...");
  RgbRing.setPattern(RingPattern::Scanning);
  WifiLed.setState(WifiLedState::Scanning);

  const uint32_t start = millis();
  while (millis() - start < kDurationMs) {
    RgbRing.update();
    WifiLed.update();
    delay(20);
  }

  gActive = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_set_promiscuous_rx_cb(nullptr);

  Serial.printf("Done. EAPOL=%lu mgmt=%lu\n", (unsigned long)gEapol, (unsigned long)gMgmt);
  logSummaryToSd();
  RgbRing.setPattern(RingPattern::Success);
  Buzzer.playSuccess();
  sessionDone = true;
}

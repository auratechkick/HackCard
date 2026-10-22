# pragma once

// HackCard RP2350 hardware map (verified production pinout)

// ---------------- I2C / PN532 NFC ----------------
// GPIO6/GPIO7 are I2C1 pins on RP2350 — use Wire1, not Wire
#define HC_SDA_PIN        6
#define HC_SCL_PIN        7
#define HC_NFC_IRQ_PIN    11

// ---------------- RGB LEDs (12-LED WS2812 strip) ----------------
#define HC_STRIP_LED_PIN   24
#define HC_STRIP_LED_COUNT 12
#define HC_STATUS_LED_INDEX 0   // first pixel used as status indicator

// ---------------- Buzzer (NPN switched) ----------------
#define HC_BUZZER_PIN      9

// ---------------- SD Card (SPI1) ----------------
// GPIO25-28 are SPI1 pins on RP2350A — use SPI1, not SPI
#define HC_SD_CS           25
#define HC_SD_MOSI         27
#define HC_SD_CLK          26
#define HC_SD_MISO         28

// ---------------- User button (active low, pull-up) ----------------
#define HC_BUTTON_PIN      2

// BOOT (QSPI-3) and RUN are hardware-only — manual QC, not GPIO tests.

// ---------------- Test timing ----------------
#define HC_NFC_WAIT_MS        30000
#define HC_BUTTON_WAIT_MS     15000
#define HC_TEST_VERSION       "1.0.4"

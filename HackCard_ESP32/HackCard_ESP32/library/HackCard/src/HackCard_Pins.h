#pragma once

// ═══════════════════════════════════════════════════════
//  HackCard ESP32-S3 — Hardware pin map
//  Matches firmware config/board_config.h
// ═══════════════════════════════════════════════════════

#define HACKCARD_BOARD_NAME       "HackCard ESP32-S3"
#define HACKCARD_BOARD_MCU        "ESP32-S3FH4R2"
#define HACKCARD_LIBRARY_VERSION  "1.0.0"

#define PIN_BOOT                  0

#define PIN_BUZZER                37

#define PIN_RGB_WIFI              38
#define RGB_WIFI_COUNT            1

#define PIN_RGB_RING              45
#define RGB_RING_COUNT            12

#ifndef RGB_USE_GRB
#define RGB_USE_GRB               1
#endif

#define PIN_NFC_SDA               8
#define PIN_NFC_SCL               18
#define PIN_NFC_IRQ               17
#define NFC_I2C_ADDRESS           0x24

#define PIN_SD_CS                 10
#define PIN_SD_MOSI               11
#define PIN_SD_CLK                12
#define PIN_SD_MISO               13

#define BOOT_LONG_PRESS_MS        1500
#define BOOT_DOUBLE_TAP_MS        400
#define BOOT_SAFE_MODE_MS         5000

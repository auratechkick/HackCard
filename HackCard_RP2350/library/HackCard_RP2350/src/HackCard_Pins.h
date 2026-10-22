#pragma once

// ═══════════════════════════════════════════════════════
//  HackCard RP2350 — Hardware pin map
//  Matches firmware config/board_config.h
//  Verified on production-test firmware 1.0.4
// ═══════════════════════════════════════════════════════

#define HACKCARD_BOARD_NAME       "HackCard RP2350"
#define HACKCARD_BOARD_MCU        "RP2350"
#define HACKCARD_LIBRARY_VERSION  "0.1.0"

// User button (active low, pull-up). BOOT and RUN are hardware-only.
#define PIN_BUTTON                2

#define PIN_BUZZER                9

// One 12-LED WS2812 strip. There is no separate radio LED.
#define PIN_RGB_RING              24
#define RGB_RING_COUNT            12
#define RGB_STATUS_INDEX          0

#ifndef RGB_USE_GRB
#define RGB_USE_GRB               1
#endif

// PN532 on I2C1. Use Wire1, not Wire.
#define PIN_NFC_SDA               6
#define PIN_NFC_SCL               7
#define PIN_NFC_IRQ               11
#define NFC_I2C_ADDRESS           0x24

// microSD on SPI1. Use SPI1, not SPI.
#define PIN_SD_CS                 25
#define PIN_SD_CLK                26
#define PIN_SD_MOSI               27
#define PIN_SD_MISO               28

#define BUTTON_LONG_PRESS_MS      1500
#define BUTTON_DOUBLE_TAP_MS      400

// Gesture timing names kept so ported sketches can share the HAL.
#define BOOT_LONG_PRESS_MS        BUTTON_LONG_PRESS_MS
#define BOOT_DOUBLE_TAP_MS        BUTTON_DOUBLE_TAP_MS

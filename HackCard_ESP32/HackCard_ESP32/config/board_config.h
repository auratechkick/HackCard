#pragma once

// ═══════════════════════════════════════════════════════
//  HackCard — Hardware Configuration (ESP32-S3FH4R2)
//  Edit only if you have a custom PCB revision.
// ═══════════════════════════════════════════════════════

#define BOARD_NAME              "HackCard ESP32-S3"
#define BOARD_MCU               "ESP32-S3FH4R2"
#define FIRMWARE_VERSION        "1.0.2"
#define FLASH_SIZE_MB           4
#define PSRAM_SIZE_MB           2

// ── Button (BOOT only — RESET is hardware) ───────────
#define PIN_BOOT                0

// ── Buzzer ─────────────────────────────────────────────
#define PIN_BUZZER              37

// ── RGB LEDs (WS2812) ──────────────────────────────────
#define PIN_RGB_WIFI            38      // 1 LED at WiFi symbol
#define RGB_WIFI_COUNT          1

#define PIN_RGB_RING            45      // 12 LEDs in circle
#define RGB_RING_COUNT          12

#define RGB_USE_GRB             1       // 1=GRB (most WS2812), 0=RGB

// ── PN532 NFC (I2C) ────────────────────────────────────
#define PIN_NFC_SDA             8
#define PIN_NFC_SCL             18
#define PIN_NFC_IRQ             17
#define NFC_I2C_ADDRESS         0x24

// ── SD Card (SPI) ────────────────────────────────────────
#define PIN_SD_CS               10
#define PIN_SD_MOSI             11
#define PIN_SD_CLK              12
#define PIN_SD_MISO             13

// ── Power ────────────────────────────────────────────────
#define USB_POWER_ONLY          1

// ── HackCard Wi-Fi bring-up ─────────────────────────────
// Start Wi-Fi before RGB/buzzer/SD. Keep LEDs off until AP is stable.
#define WIFI_PERIPHERAL_DELAY_MS  3000
#define SD_SCAN_DELAY_MS          8000
#define BOOT_LONG_PRESS_MS      1500
#define BOOT_DOUBLE_TAP_MS      400
#define BOOT_SAFE_MODE_MS       5000

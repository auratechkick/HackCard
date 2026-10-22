#pragma once

// ═══════════════════════════════════════════════════════
//  HackCard — Hardware Configuration (RP2350)
//  Edit only if you have a custom PCB revision.
//  Pin map verified by production-test firmware 1.0.4.
//  Keep in sync with library/HackCard/src/HackCard_Pins.h
// ═══════════════════════════════════════════════════════

#define BOARD_NAME              "HackCard RP2350"
#define BOARD_MCU               "RP2350"
#define FIRMWARE_VERSION        "0.1.0"
#define FLASH_SIZE_MB           2

// User button (active low). BOOT (QSPI) and RUN are hardware-only.
#define PIN_BUTTON              2

#define PIN_BUZZER              9

// 12-LED WS2812 strip. No separate radio LED on this board.
#define PIN_RGB_RING            24
#define RGB_RING_COUNT          12
#define RGB_STATUS_INDEX        0
#define RGB_USE_GRB             1

// PN532 NFC on I2C1
#define PIN_NFC_SDA             6
#define PIN_NFC_SCL             7
#define PIN_NFC_IRQ             11
#define NFC_I2C_ADDRESS         0x24

// microSD on SPI1
#define PIN_SD_CS               25
#define PIN_SD_CLK              26
#define PIN_SD_MOSI             27
#define PIN_SD_MISO             28

#define USB_POWER_ONLY          1

#define BUTTON_LONG_PRESS_MS    1500
#define BUTTON_DOUBLE_TAP_MS    400

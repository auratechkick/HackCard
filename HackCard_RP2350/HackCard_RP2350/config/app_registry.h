#pragma once

// Feature compile switches — 1 = included, 0 = omitted.
// RP2350 has no radio, so every Wi-Fi and BLE app stays off.

#define CLI_SERIAL_ENABLED      1
#define APP_SYSTEM_DIAG         1
#define APP_SYSTEM_WEB          0

#define APP_PRO_PROFILE         1
#define APP_DEMO_LED_BUZZER     1
#define APP_SYSTEM_LOGS         1

#define APP_NFC_READ            1
#define APP_NFC_INFO            1
#define APP_NFC_DUMP            1
#define APP_NFC_WRITE           1
#define APP_NFC_CLONE           0
#define APP_NFC_KILL            0
#define APP_NFC_CLASSIC         0

#define APP_WIFI_SCANNER        0
#define APP_WIFI_PORTAL         0
#define APP_WIFI_AP_CONTROL     0
#define APP_WIFI_BEACON         0
#define APP_WIFI_EVIL_TWIN      0
#define APP_WIFI_TRAINING       0
#define APP_WIFI_MONITOR        0
#define APP_WIFI_RECON          0
#define APP_WIFI_KARMA          0
#define APP_WIFI_CAPTURE        0
#define APP_WEB_UI              0

#define APP_BLE                 0
#define APP_USB_HID             1
#define APP_SD                  1

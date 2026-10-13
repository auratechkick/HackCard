#pragma once

// Feature compile switches — 1 = included in release build, 0 = omitted (saves flash).
// See RELEASE.md and docs/ for the default 1.0.0 feature set.

// Set to 1 while debugging on USB Serial; backers use web terminal
#define CLI_SERIAL_ENABLED      0
#define APP_SYSTEM_DIAG         1
#define APP_SYSTEM_WEB          1

#define APP_PRO_PROFILE         1   // contact profile page + vCard
#define APP_DEMO_LED_BUZZER     1   // LED/buzzer demo apps
#define APP_SYSTEM_LOGS         1   // activity log viewer
#define APP_MODES               1   // preset operating modes

#define APP_NFC_READ            1   // PN532 read UID app
#define APP_NFC_INFO            1   // tag family + NDEF preview
#define APP_NFC_DUMP            1   // Type 2 page dump to storage
#define APP_NFC_WRITE           1   // NDEF URL write + lab erase
#define APP_NFC_CLONE           1   // Type 2 restore / clone from dump
#define APP_NFC_KILL            1   // Permanently lock Type 2 tag
#define APP_NFC_CLASSIC         1   // Mifare Classic dump/clone/keyscan

#define APP_WIFI_SCANNER        1   // Wi-Fi scan + connect + /wifi page
#define APP_WIFI_PORTAL         1   // Captive portal + /portal landing
#define APP_WIFI_AP_CONTROL     1   // AP settings page + live apply
#define APP_WIFI_BEACON         1   // Lab beacon broadcast demo
#define APP_WIFI_EVIL_TWIN      1   // Lab evil-twin simulation (time-limited)
#define APP_WIFI_TRAINING       1   // Training / phishing awareness portal
#define APP_WIFI_MONITOR        1   // Probe sniff + limited deauth demo
#define APP_WIFI_RECON          0   // Disabled — use serial CLI if re-enabled in firmware
#define APP_WIFI_KARMA          1   // Karma probe-response lure (lab, PIN-gated)
#define APP_WIFI_CAPTURE        1   // EAPOL/handshake capture + PCAP export
#define APP_WEB_UI              1   // Lab Console SPA — embedded UI; optional LittleFS /www override

#define APP_BLE                 1   // BLE scan + lab advertiser + contact card
#define APP_USB_HID             1   // USB keyboard HID lab payloads

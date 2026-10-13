#pragma once

// ═══════════════════════════════════════════════════════
//  HackCard — YOUR PERSONAL SETTINGS
//  Edit this file, then re-upload firmware.
//  Works without SD card (Flash Mode). SD adds export capacity (Full Mode).
// ═══════════════════════════════════════════════════════

// ── Your Identity (simple contact card) ────────────────
#define USER_NAME               "Your Name"
#define USER_TITLE              "Security Researcher"
#define USER_COMPANY            "Your Company"
#define USER_EMAIL              "you@example.com"
#define USER_PHONE              "+1 000 000 0000"
#define USER_WEBSITE            "https://yoursite.com"
#define USER_LINKEDIN           "https://linkedin.com/in/yourprofile"
#define USER_GITHUB             "https://github.com/yourprofile"
#define USER_BIO                "Ethical hacking & embedded security."

// ── Wi-Fi Hardware Test (set to 1 while debugging PCB antenna) ─
// Set to 1 only while debugging Wi-Fi PCB antenna
#define WIFI_HARDWARE_TEST_MODE   0
#define WIFI_TEST_STA_SSID        "Hotspot"
#define WIFI_TEST_STA_PASSWORD    "12345678"
#define WIFI_TEST_CONNECT_TIMEOUT 30000   // ms

// ── Wi-Fi Access Point ─────────────────────────────────
// HackCard AP is 2.4 GHz only (ESP32 cannot do 5 GHz).
// Phones must scan 2.4 GHz networks to see this SSID.
#define AP_SSID                 "HackCard-Setup"
#define AP_PASSWORD             "hackcard2026"   // min 8 characters
#define AP_CHANNEL              0                // 0 = auto

// ── Defaults ───────────────────────────────────────────
#define DEFAULT_RING_BRIGHTNESS 80       // 10–100, saved in /config/hackcard.json
#define DEFAULT_BUZZER_VOLUME   80       // 10–100, PWM duty for buzzer
#define BOOT_ANIMATION          true
#define BUZZER_ENABLED          true
#define ACTIVITY_LOGGING        true
#define CAPTIVE_PORTAL_ENABLED  true

// ── Security (change before shipping to clients) ───────
#define LAB_MODE_PIN            "1234"
#define VAULT_PIN               "0000"

// ── Storage preferences ────────────────────────────────
#define SD_AUTO_CREATE_FOLDERS  true
#define SD_SHOW_WARNINGS        true

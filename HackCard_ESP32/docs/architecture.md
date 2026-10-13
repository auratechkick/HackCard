# Firmware architecture (1.0.0)

For developers extending **`HackCard_ESP32.ino`** or calling the device REST API.

## High-level layout

```
HackCard_ESP32/
├── HackCard_ESP32.ino      setup/loop, deferred RGB/SD init
├── config/
│   ├── board_config.h      pins, FIRMWARE_VERSION
│   ├── user_config.h       owner profile, AP, PINs (edit before ship)
│   └── app_registry.h      compile-time feature flags (APP_*)
├── src/
│   ├── core/               Storage, Config, Log, DeviceMode, CLI
│   ├── platform/hal/       PN532, RGB, Wi-Fi AP, USB HID, …
│   ├── apps/               wifi/, nfc/, usb/, system/, demo/
│   └── apps/system/        WebStatusServer, WebUiEmbedded.*
└── library/HackCard/       slim HAL + examples (no web stack)
```

## Boot sequence (simplified)

1. USB Serial @ 115200, optional USB HID init  
2. `Storage.begin()` — LittleFS; mount SD if present  
3. `Config.begin()` — load `/config/hackcard.json` (flash first)  
4. `WebStatus.begin()` — soft AP + HTTP server + embedded UI  
5. After delay — RGB ring, buzzer, NFC init (`initDeferredPeripherals`)

Wi-Fi starts **before** heavy peripherals to improve AP stability on the PCB.

## Device modes

| Mode | Behavior |
|------|----------|
| **Business** | Profile HTML at `/`, lab routes gated |
| **Lab** | Full Lab Console; `/api/lab/unlock` with PIN |

Managed by `DeviceModeManager` + config.

## Storage model

| Tier | Trigger | UI |
|------|---------|-----|
| FlashOnly | No SD | `storage_status`: flash; SD export APIs return unavailable |
| SdFull | SD mounted | Export paths on card under `/wifi`, `/nfc`, `/payloads` |

Web UI reads tier from **`GET /api/status`**.  
Implementation: `StorageManager` — `/www/` reads **flash only**, not SD.

## Web stack

| Piece | Role |
|-------|------|
| `WebStatusServer` | REST `/api/*`, static files, captive helpers |
| `WebUiEmbedded.*` | Generated from `hackcard-web-ui/www` via `sync-web-ui.ps1` |
| `data/www/` | Optional LittleFS override (Sketch Data Upload) |

Front-end client: `hackcard-web-ui/www/assets/js/api.js`.

## Feature flags

`config/app_registry.h` — set `APP_*` to `0` to drop apps and save flash.  
Must match what you document in release notes if you ship a slim build.

## Examples vs monolith

| | Full firmware | `library/HackCard` example |
|--|---------------|----------------------------|
| Web + API | Yes | No |
| Single app focus | All apps | One sketch |
| Best for | Product | Learning / custom firmware |

See [CUSTOM_DEVELOPMENT.md](../HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md).

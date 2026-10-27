# Changelog

All notable firmware and library releases for HackCard ESP32.

## [1.0.2] — 2026-10-22

### Added

- **Files** page in the Lab Console. Check the microSD card in the slot (type, capacity, used and free space), then open, download, or delete lab files without removing the card. Flash lab files are on the same page.
- REST routes: `GET /api/files/status`, `/api/files/list`, `/api/files/read`, `/api/files/download`, `POST /api/files/delete` (delete requires the Lab PIN).
- Example **`07_System/Factory_Reset`**. Hold BOOT for 3 seconds or type `RESET` to clear the saved AP password, lab PIN, profile, NVS backup, flash lab files, and HackCard folders on the SD card. Then upload `HackCard_ESP32.ino` again. Defaults come from `config/user_config.h`.

### Changed

- `FIRMWARE_VERSION` → `1.0.2`
- Arduino library **1.0.2** — **73** examples

## [1.0.1] — 2026-10-22 (library + docs; firmware remains 1.0.0)

### Added

- **22 Arduino examples** ported from HackCard RP2350: advanced USB HID (OS menu, desk remote, Ducky Script, presentation clicker, email/meeting/copy templates), NFC page dump to SD, RGB + button, and RealLife NFC→HID / standup / guest Wi-Fi sketches
- Library module **`HackCardUsb.h`**: `UsbHid`, `Host`, `Ducky`, `HackSd` for ESP32-S3 TinyUSB
- Script `scripts/port-rp2350-examples.ps1` to re-sync from the RP2350 tree

### Changed

- **`EXAMPLES.md`** and in-repo documentation updated for **72** total library examples
- Arduino library version **1.0.1** in `library.properties` (firmware `FIRMWARE_VERSION` unchanged)

## [1.0.0] — 2026-10-02

### Release (GitHub bundle)

- **Lab Console** embedded in firmware; optional LittleFS `/www/` override via Sketch Data Upload
- **Storage:** Flash Mode (no SD) vs Full Mode (SD exports under `/wifi`, `/nfc`, `/payloads`)
- Dashboard UI **not** read from SD `/www/` (prevents stale cards overriding firmware UI)
- Business / Lab device modes, profile portal, REST API + web terminal
- Wi‑Fi, NFC, BLE, USB HID lab apps; Monitor/Capture with SD-only export when card present
- Docs in-repo: `docs/SERIAL_AND_LOGGING.md`, `docs/STORAGE.md`, `docs/CUSTOM_DEVELOPMENT.md`, `RELEASE.md`
- Library examples index 1.0.0: ring control, karma, EAPOL capture, SD lab folders (+ 46 existing sketches)

### Changed

- `FIRMWARE_VERSION` → `1.0.0`; boot banner and `/api/status` report same string
- CLI serial input uses USB `Serial` when `CLI_SERIAL_ENABLED`

## [0.21.3] — 2026-09-10

### Added

- Full web-feature → Serial example coverage for backers (`EXAMPLES.md`)
- `Template_Custom_App` + `Web_Feature_Map` under `00_Start_Here`
- NFC labs: write text/vCard, erase, clone, kill, Classic dump/clone/keyscan
- Wi-Fi labs: STA connect, captive portal, beacon, evil-twin sim, training portal, probe sniff, deauth demo
- HID labs: open website, apps/lock, media, mouse, key press
- System: diagnostics, profile/vCard, modes, activity log; BLE GATT contact card; LED demos 1–12

### Notes

- Lab sketches gate destructive / RF demos behind an editable lab PIN
- Use only on hardware/networks/tags you own or have written permission to test

## [0.21.2] — 2026-09-10

### Added

- Public GitHub release packaging: README, LICENSE, library, examples
- Arduino library `library/HackCard` with pin map + HAL drivers
- Standalone Serial examples (basics, NFC, Wi-Fi, BLE, USB HID, SD)
- Real-life examples: conference badge tap, guest portal, BLE business card, desk status light

### Fixed

- Version messaging aligned to Phase 21 / `0.21.2`

### Notes

- Full firmware remains the web dashboard + CLI monolith (`HackCard_ESP32.ino`)
- PN532 is **reader only** (no card emulation)

## [0.21.1]

- PIN pop-up validation on `/hid`
- Editable Wi-Fi SSID, password, and Lab PIN from `/hid` and `/settings` (saved to flash)

## [0.20.0]

- Dedicated HID page with OS picker, Notepad, Lock, Type Text, Open URL

## Earlier phases

Phases 0–19 built core HAL, storage modes, NFC labs, Wi-Fi labs, BLE contact card, and the web status dashboard. See git history on [auratechkick/HackCard](https://github.com/auratechkick/HackCard) and the [ESP32 docs hub](../docs/README.md).

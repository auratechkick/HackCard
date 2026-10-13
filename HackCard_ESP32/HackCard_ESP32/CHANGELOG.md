# Changelog

All notable firmware and library releases for HackCard ESP32.

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

Phases 0–19 built core HAL, storage modes, NFC labs, Wi-Fi labs, BLE contact card, and the web status dashboard. See repository history and the [docs changelog](https://hardwarehackspace.github.io/hackcard-docs/resources/changelog/).

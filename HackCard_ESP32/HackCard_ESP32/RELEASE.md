# Release 1.0.0 — GitHub firmware bundle

## Included

- Full HackCard ESP32-S3 firmware (Lab + Business device modes)
- Embedded Lab Console web UI (sync via `scripts/sync-web-ui.ps1`)
- Arduino library + **50+ examples** under `library/HackCard/examples/` (see `EXAMPLES.md`, `docs/CUSTOM_DEVELOPMENT.md`)
- Flash **and** SD storage paths (exports SD-only)

## Not included in this repo folder

- Separate MkDocs website (optional later) — primary docs ship in repo [`docs/`](../docs/README.md)
- Pre-built binary `.bin` — build from source in Arduino IDE or CI

## Pre-release checklist

- [ ] Change default `LAB_MODE_PIN` and `AP_PASSWORD` in `config/user_config.h`
- [ ] Run `scripts/sync-web-ui.ps1` after any UI edit
- [ ] Upload firmware + verify http://192.168.4.1/ (Flash Mode and Full Mode)
- [ ] Confirm SD export buttons disabled without card, enabled with card
- [ ] Serial 115200 boot banner and web Terminal log
- [ ] Upload 3–5 library examples (Basics, NFC, Wi-Fi, HID) — headers say `release 1.0.0 example`

## Version string

Single source of truth: **`FIRMWARE_VERSION`** in `config/board_config.h` (also exposed in `/api/status` as `firmware`).

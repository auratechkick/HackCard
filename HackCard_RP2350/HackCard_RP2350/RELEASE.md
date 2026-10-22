# Release 0.1.0 — GitHub bundle

Serial-lab firmware and Arduino library for **HackCard RP2350** (no Wi‑Fi / BLE / web UI).

## Included

- Serial lab firmware (`HackCard_RP2350.ino`, `src/SerialLab.cpp`)
- Arduino library **`HackCard_RP2350`** with **40** examples — [EXAMPLES.md](../library/HackCard_RP2350/EXAMPLES.md)
- Production test sketch under `../sketches/HackCard_Production_Test/`
- Documentation under `../docs/`

## Out of scope (use HackCard ESP32 instead)

- Lab Console at `http://192.168.4.1`
- Wi‑Fi / BLE lab apps
- On-device JSON settings in flash filesystem (use `config/user_config.h`)

## Pre-release checklist

- [ ] Board: **Generic RP2350**, **2MB (no FS)**, **USB Stack: Pico SDK**
- [ ] Run `scripts/install-library.ps1` and confirm examples appear in IDE
- [ ] Upload firmware → Serial **115200** → `help`, `nfc`, `sd`
- [ ] Upload 3–5 library examples (Basics, NFC, HID)
- [ ] Optional: run production test sketch on hardware QC bench

## Version string

**`FIRMWARE_VERSION`** in `config/board_config.h` (shown on serial boot banner).

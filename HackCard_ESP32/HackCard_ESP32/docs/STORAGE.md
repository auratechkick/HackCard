# Storage — Flash Mode vs Full Mode

## Tiers

| Tier | Label in UI | When |
|------|-------------|------|
| Flash only | **Flash Mode** | No microSD, or card not yet mounted (~8 s after boot) |
| SD mounted | **Full Mode** | Valid card detected on SPI SD pins |

Core lab tools work in **both** modes.

## What uses flash only

- Device config `/config/hackcard.json` (+ NVS backup)
- **Lab Console UI** — embedded in firmware (`WebUiEmbedded.*`); optional LittleFS `/www/` override via Sketch Data Upload
- NFC/Wi‑Fi data (may **mirror** to SD when card present for `/wifi/` paths)

## SD-only / SD-export features

These require **`sd_ready`** (Full Mode):

- **Export → SD** buttons (training, probes, PCAP, NFC history/dumps)
- **Save to SD** (HID payloads, optional SD-only sniff/capture/NFC checkboxes)

API error when no card: *Insert an SD card (Full Mode). Export to SD is not available without a card.*

## Folders created on SD

On first mount, firmware creates (if `SD_AUTO_CREATE_FOLDERS` is true):

| Folder | Use |
|--------|-----|
| `config/` | Optional config copy |
| `logs/` | Extended logs |
| `nfc/` | Dumps; `nfc/exports/` for SD exports |
| `wifi/` | Lab data; `wifi/exports/` for debrief files |
| `ble/` | BLE lab exports (future-friendly) |
| `payloads/` | HID scripts saved to SD |

**Not created for dashboard:** UI is **not** read from SD `/www/` (avoids stale web UI on card).

## User workflow

- **No SD:** use dashboard + HTTP download where available; keep capture/sniff sessions short.
- **With SD:** use Export → SD after labs; copy `exports` folders to a PC for debrief.

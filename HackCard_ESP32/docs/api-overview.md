# REST API overview

The Lab Console talks to the device over **HTTP** on the AP IP (default **`192.168.4.1`**).  
Most routes are **POST** with `application/x-www-form-urlencoded` unless noted.

Implementation: `HackCard_ESP32/src/apps/system/WebStatusServer.cpp`  
Client reference: `hackcard-web-ui/www/assets/js/api.js`

> **Security model:** You are on the device’s own AP. There is no public internet exposure by default. Still change default PINs and AP passwords before untrusted environments.

---

## System

| Method | Path | Purpose |
|--------|------|---------|
| GET | `/api/status` | Firmware version, SD ready, storage tier, mode, profile hints |
| POST | `/api/setup` | Initial / setup fields |
| POST | `/api/profile` | Profile / vCard fields |
| POST | `/api/lab/unlock` | `pin=` → Lab mode |
| POST | `/api/cli` | Web CLI command (`cmd=`) |
| GET | `/api/log/serial` | Serial log buffer (`since=`) |
| GET | `/api/logs` | Activity log |
| POST | `/api/diag/run` | Diagnostics |
| POST | `/api/settings` | Device settings |
| POST | `/api/apps` | Ring demo / peripheral apps |
| GET | `/api/files/status` | SD and flash ready, card type, capacity. `probe=1` also measures used and free space |
| GET | `/api/files/list` | `volume=sd\|flash` and `path=` — one folder |
| GET | `/api/files/read` | Text preview (first 4 KB) or `binary: true` |
| GET | `/api/files/download` | Full file attachment |
| POST | `/api/files/delete` | `volume`, `path`, `pin` — Lab PIN required |

---

## Wi-Fi

| Area | Examples |
|------|----------|
| Scan / STA | `/api/wifi/scan`, `/api/wifi/sta`, `/api/wifi/last` |
| AP | `/api/wifi/ap` GET/POST |
| Beacon | `/api/wifi/beacon`, `/start`, `/stop` |
| Evil twin | `/api/wifi/twin/*` |
| Training | `/api/wifi/training/*` |
| Monitor | `/api/wifi/monitor/*` (sniff, deauth, probes, export SD) |
| Karma | `/api/wifi/karma/*` |
| Capture | `/api/wifi/capture/*` (EAPOL PCAP, export SD) |

Lab routes typically require `pin=` in the POST body (same Lab PIN as UI).

---

## NFC

| Path | Purpose |
|------|---------|
| `/api/nfc/read`, `/info`, `/dump`, `/write` | Core tag ops |
| `/api/nfc/erase`, `/clone`, `/kill` | Type 2 lab (PIN) |
| `/api/nfc/classic/*`, `/api/nfc/keys/scan` | Mifare Classic |
| `/api/nfc/history`, `…/export/sd` | History + SD export |

---

## BLE

| Path | Purpose |
|------|---------|
| `/api/ble/scan`, `/api/ble/scan/last` | Scanner |
| `/api/ble/advertise/start`, `/stop` | Advertiser |
| `/api/ble/contact/*` | Contact card GATT |

---

## HID

| Path | Purpose |
|------|---------|
| `/api/hid/status`, `/device`, `/os` | Host / OS selection |
| `/api/hid/run` | Run built-in payload |
| `/api/hid/payloads`, `/payload`, `/payload/save` | Script storage |
| `/api/hid/arm` | Arm payload on long-press (device button) |

Host PC must be connected via USB; firmware uses TinyUSB HID.

---

## SD export pattern

Routes ending in **`/export/sd`** return an error JSON if no SD is mounted (Flash Mode).  
UI disables buttons using `/api/status` → `sd_ready`.

---

## Extending the API

1. Add handler in `WebStatusServer.cpp` (follow existing PIN checks).  
2. Add client method in `api.js`.  
3. Wire UI in `dashboard.js`.  
4. Run `sync-web-ui.ps1` and reflash.

For automation without the UI, call the same URLs from curl on a laptop joined to the AP.

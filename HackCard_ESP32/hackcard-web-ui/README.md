# HackCard Lab Console (web UI)

Source for the HackCard **Lab Console** — HTML/CSS/JS served by firmware at `http://192.168.4.1/`.

**End-user guide (pages, PIN, mobile UI):** [`../docs/lab-console-user-guide.md`](../docs/lab-console-user-guide.md)  
**Developer API list:** [`../docs/api-overview.md`](../docs/api-overview.md)

## Edit and ship in firmware

```powershell
cd HackCard_ESP32\scripts
.\sync-web-ui.ps1
```

Rebuild and upload **`HackCard_ESP32.ino`**. The UI is baked into `WebUiEmbedded.*` inside the sketch.

Optional: **Tools → ESP32 Sketch Data Upload** after sync to mirror files into on-chip LittleFS (`HackCard_ESP32/data/www/`).

**Do not** copy `www/` onto the microSD card for the dashboard — firmware ignores SD `/www/` so old cards do not override the current UI.

## Layout

| Path | Role |
|------|------|
| `www/index.html` | Lab Console shell |
| `www/assets/js/dashboard.js` | Pages, Wi‑Fi/NFC labs, storage-aware export UI |
| `www/assets/js/api.js` | REST client for `/api/*` |
| `www/assets/css/dashboard.css` | Theme (desktop sidebar + mobile tabs) |

## Local preview

Serve `www/` with any static file server. API calls must target a live HackCard (or a dev proxy to `192.168.4.1`).

## Device modes

- **Business** — profile portal until Lab PIN unlock
- **Lab** — full console (Monitor, Capture, HID, training scenarios)

Lab PIN and storage tier (`flash` vs `full`) come from `/api/status`.

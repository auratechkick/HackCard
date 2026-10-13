# LittleFS data — Lab Console (optional)

The Lab Console is **embedded in firmware** after you run `scripts/sync-web-ui.ps1` and rebuild. **No SD card and no LittleFS upload are required** for the dashboard.

Use this folder when you want to refresh the web UI **without** reflashing the full application binary:

1. Run sync after UI edits:

```powershell
cd HackCard_ESP32\scripts
.\sync-web-ui.ps1
```

2. Arduino IDE: **Tools → ESP32 Sketch Data Upload**
3. Open `http://192.168.4.1/`

**SD card:** the dashboard is **not** loaded from SD `/www/` (see `docs/STORAGE.md`). Edit UI source in `hackcard-web-ui/www/` at the repo root.

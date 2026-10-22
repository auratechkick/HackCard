# Repository documentation assets

Files in **`assets/`** and **`index.html`** support the [root README](../README.md) and optional **GitHub Pages** (`Settings ? Pages ? Deploy from branch ? /docs`).

| File | Source |
|------|--------|
| `hero.png` | Lab Console dashboard (`github-preview/dashboard.html`) |
| `gallery-dashboard.png` | Same as hero (status overview) |
| `gallery-nfc.png` | NFC Reader page |
| `gallery-hid.png` | USB HID lab |
| `gallery-profile.png` | Business mode digital profile |
| `gallery-rp2350.png` | RP2350 serial lab mock terminal |

**Regenerate screenshots:** serve `HackCard_ESP32/hackcard-web-ui/www/` locally (`python -m http.server`), open `github-preview/*.html` at **1280px** width, capture PNGs into `docs/assets/`.

Product documentation: **[HackCard_ESP32/docs](../HackCard_ESP32/docs/README.md)** · **[HackCard_RP2350/docs](../HackCard_RP2350/docs/README.md)**.

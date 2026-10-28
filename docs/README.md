# Repository documentation assets

Files here support the [root README](../README.md) on **GitHub Pages** (`main` branch ? **`/docs`** folder).

| Site | URL |
|------|-----|
| Gallery landing | https://auratechkick.github.io/HackCard/ |
| **ESP32 user guide (MkDocs)** | https://auratechkick.github.io/HackCard/esp32/ |

## Screenshots (`assets/`)

| File | Source |
|------|--------|
| `hero.png` | Lab Console dashboard (`github-preview/dashboard.html`) |
| `gallery-*.png` | Lab Console UI captures |

Regenerate: serve `HackCard_ESP32/hackcard-web-ui/www/` locally, open `github-preview/*.html` at **1280px**, save PNGs to `docs/assets/`.

## ESP32 guide (`esp32/`)

Built from **`HackCard_ESP32/user-guide/`** (MkDocs Material):

```powershell
cd HackCard_ESP32\user-guide
pip install -r requirements-docs.txt
mkdocs build -d ../../docs/esp32
```

Pushes that only change `user-guide/` can rebuild via GitHub Actions (`.github/workflows/build-esp32-docs.yml`).

Product markdown in-repo: **[HackCard_ESP32/docs](../HackCard_ESP32/docs/README.md)** · **[HackCard_RP2350/docs](../HackCard_RP2350/docs/README.md)**.

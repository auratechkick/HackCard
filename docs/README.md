# Repository documentation assets

GitHub Pages serves this **`/docs`** folder from **`main`**.

| Site | URL |
|------|-----|
| Gallery landing | https://auratechkick.github.io/HackCard/ |
| **ESP32 user guide (MkDocs)** | https://auratechkick.github.io/HackCard/esp32/ |
| **RP2350 user guide (MkDocs)** | https://auratechkick.github.io/HackCard/rp2350/ |

## MkDocs sources

| Board | Source | Build output |
|-------|--------|--------------|
| ESP32 | `HackCard_ESP32/user-guide/` | `docs/esp32/` |
| RP2350 | `HackCard_RP2350/user-guide/` | `docs/rp2350/` |

```powershell
cd HackCard_ESP32\user-guide
pip install -r requirements-docs.txt
mkdocs build -d ../../docs/esp32

cd ..\..\HackCard_RP2350\user-guide
mkdocs build -d ../../docs/rp2350
```

CI: `.github/workflows/build-user-guides.yml` rebuilds both on push to either `user-guide/` tree.

## Screenshots (`assets/`)

Gallery PNGs for the root README — regenerate from `HackCard_ESP32/hackcard-web-ui/www/github-preview/`.

In-repo markdown: **[HackCard_ESP32/docs](../HackCard_ESP32/docs/README.md)** · **[HackCard_RP2350/docs](../HackCard_RP2350/docs/README.md)**.

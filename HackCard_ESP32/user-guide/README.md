# HackCard ESP32 — user guide (MkDocs)

**Published:** [https://auratechkick.github.io/HackCard/esp32/](https://auratechkick.github.io/HackCard/esp32/)

Source lives here; static HTML is built into repo **`docs/esp32/`** for GitHub Pages.

**RP2350 guide:** [HackCard_RP2350/user-guide](https://github.com/auratechkick/HackCard/tree/main/HackCard_RP2350/user-guide) → https://auratechkick.github.io/HackCard/rp2350/

## Preview locally

```powershell
cd HackCard_ESP32\user-guide
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -r requirements-docs.txt
mkdocs serve -a 127.0.0.1:8000
```

Keep that terminal **open** while you read the docs. If the browser shows **Error -102**, the server is not running (connection refused).

Open **http://127.0.0.1:8000/HackCard/esp32/** (MkDocs uses the `site_url` path when serving locally).

**Windows shortcut:** double-click `serve-docs.cmd` in this folder (same as the command above).

Build static HTML (optional):

```powershell
mkdocs build -d ../../docs/esp32
# commits to docs/esp32/ for GitHub Pages
```

## Source of truth

Firmware and settings live in the parent tree:

- `../HackCard_ESP32/` — main sketch, `config/user_config.h`, Lab Console firmware
- `../docs/` — existing markdown docs in the monorepo
- `../hackcard-web-ui/www/` — Lab Console UI source

Documentation version: **firmware 1.0.2** (`config/board_config.h`).

## Next phases (after your review)

- Software, Features, Tutorials, Troubleshooting (full nav like the old site)
- RP2350 getting-started companion guide
- Publish under `auratechkick/HackCard` GitHub Pages

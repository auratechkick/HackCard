# HackCard RP2350 — user guide (MkDocs)

**Published:** [https://auratechkick.github.io/HackCard/rp2350/](https://auratechkick.github.io/HackCard/rp2350/)

Source: this folder. Static HTML: **`docs/rp2350/`** on GitHub Pages.

## Preview locally

```powershell
cd HackCard_RP2350\user-guide
pip install -r requirements-docs.txt
mkdocs serve -a 127.0.0.1:8001
```

Open **http://127.0.0.1:8001/HackCard/rp2350/** (use port 8001 if ESP32 guide uses 8000).

Or run **`serve-docs.cmd`**.

## Rebuild for GitHub Pages

```powershell
mkdocs build -d ../../docs/rp2350
```

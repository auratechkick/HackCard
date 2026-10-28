# Next steps

You flashed firmware and opened the Lab Console — here is how to go deeper without leaving the monorepo.

---

## Learn one feature at a time (Arduino examples)

The **HackCard** library ships **73** sketches — each focuses on one lab or real-life demo.

→ **[Arduino example library](arduino-examples.md)** (how to run, folder map, Lab Console mapping)  
→ **[Write your own sketch](write-your-own-sketch.md)** (template, `#include <HackCard.h>`, HID helpers)

Full index on GitHub: [EXAMPLES.md](https://github.com/auratechkick/HackCard/blob/main/HackCard_ESP32/HackCard_ESP32/library/HackCard/EXAMPLES.md)

---

## In-repo documentation (markdown)

| Topic | Path in monorepo |
|-------|------------------|
| Docs hub | `HackCard_ESP32/docs/README.md` |
| Lab Console detail | `HackCard_ESP32/docs/lab-console-user-guide.md` |
| Storage / SD folders | `HackCard_ESP32/HackCard_ESP32/docs/STORAGE.md` |
| Wi-Fi issues | `HackCard_ESP32/HackCard_ESP32/docs/WIFI_TROUBLESHOOTING.md` |
| Custom firmware apps | `HackCard_ESP32/HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md` |
| REST API | `HackCard_ESP32/docs/api-overview.md` |
| Changelog | `HackCard_ESP32/HackCard_ESP32/CHANGELOG.md` |

---

## microSD folder layout (Full Mode)

When `SD_AUTO_CREATE_FOLDERS` is enabled, firmware creates lab export paths such as:

| Path | Typical content |
|------|-----------------|
| `/nfc/exports/` | NFC dumps |
| `/wifi/exports/` | Scan logs, captures |
| `/logs/` | Activity CSV |
| `/config/hackcard.json` | Optional portable overrides |

Use Lab Console **Files** to browse without removing the card.

---

## Factory reset

To clear saved AP password, Lab PIN, profile, and lab files back to **`user_config.h`**:

1. Upload **Examples → HackCard → 07_System → Factory_Reset**
2. Confirm on device (BOOT / RESET as prompted)
3. Flash **`HackCard_ESP32.ino`** again

---

## Troubleshooting quick table

| Symptom | Check |
|---------|--------|
| No COM port | Data cable; driver; **USB CDC On Boot: Enabled** |
| Wi-Fi SSID missing | 2.4 GHz scan; read SSID from Serial banner |
| Old UI after update | Reflash firmware — UI is embedded, not on SD |
| HID does nothing | **USB-OTG TinyUSB**; cable to PC; text field focused |
| Export buttons grey | Insert FAT32 SD; wait for **Full Mode** on dashboard |

---

## HackCard RP2350 (serial lab)

No Wi-Fi or Lab Console — USB serial @ **115200** and **40** Arduino examples:

**[RP2350 user guide](https://auratechkick.github.io/HackCard/rp2350/)**

---

## Authorized use

Before classroom or client demos: [Authorized use](../resources/authorized-use.md).

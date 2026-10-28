# Next steps

---

## In-repo markdown

| Topic | Path |
|-------|------|
| Docs hub | `HackCard_RP2350/docs/README.md` |
| Serial commands | `HackCard_RP2350/docs/SERIAL_CONSOLE.md` |
| Pin map | `HackCard_RP2350/docs/hardware.md` |
| Architecture | `HackCard_RP2350/docs/architecture.md` |
| Changelog | `HackCard_RP2350/HackCard_RP2350/CHANGELOG.md` |

---

## Factory reset (SD lab files)

1. **Examples → HackCard RP2350 → 06_System → Factory_Reset**
2. Upload; Serial **115200**
3. Hold **user button** 3 s or type **`RESET`**
4. Re-upload **`HackCard_RP2350.ino`**

Removes `/nfc`, `/ducky`, `/payloads`, `/logs`, `/hackcard` on SD — **not** your `user_config.h` text until you edit and re-flash.

---

## Production QC

`sketches/HackCard_Production_Test/` — bench validation before shipping boards.

---

## ESP32 sibling

Full **Lab Console**, Wi-Fi, BLE, **73** examples:

**[HackCard ESP32 user guide](https://auratechkick.github.io/HackCard/esp32/)**

---

## Authorized use

[Authorized use](../resources/authorized-use.md)

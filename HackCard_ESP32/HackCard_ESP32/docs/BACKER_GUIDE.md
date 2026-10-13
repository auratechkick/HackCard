# HackCard backer guide

Quick reference after you flash firmware **1.0.0**.

**New owner?** Start with the full walkthrough: [Getting started](../../docs/getting-started.md)  
**Web UI tour:** [Lab Console user guide](../../docs/lab-console-user-guide.md)

---

## Personalize in three steps

1. Open [`config/user_config.h`](../config/user_config.h)
2. Set your name, contact links, **AP SSID/password**, and **Lab PIN**
3. Upload [`HackCard_ESP32.ino`](../HackCard_ESP32.ino) from Arduino IDE

No microSD required for the dashboard — the Lab Console is **embedded in firmware**.

---

## Without microSD (Flash Mode)

- Boot animation, RGB ring, Wi-Fi status LED, buzzer  
- **Lab Console** at `http://192.168.4.1/` (Business profile + Lab unlock)  
- NFC / Wi-Fi / BLE / HID labs (exports to SD disabled until a card is present)  
- USB Serial log @ **115200** and **Terminal** page in the web UI  

---

## With microSD (Full Mode)

Insert a **FAT32** card — firmware switches to **Full Mode**:

- Config may sync with `/config/hackcard.json` on card  
- Activity log `/logs/activity.csv`  
- **Export to SD** for Wi-Fi monitor, NFC dumps, capture PCAP, payloads  

Folder layout: [STORAGE.md](STORAGE.md).  
**Do not** put the web UI on SD `/www/` — firmware ignores it.

---

## Web dashboard

1. Power via USB-C  
2. Join the HackCard Wi-Fi (2.4 GHz — see [WIFI_TROUBLESHOOTING.md](WIFI_TROUBLESHOOTING.md))  
3. Open **`http://192.168.4.1/`**  
4. Enter **Lab PIN** to unlock full labs  
5. **Terminal** page shows live device log (similar to Serial Monitor)

---

## BOOT button

| Gesture | Action (full firmware) |
|---------|-------------------------|
| Short press | Advance ring menu pointer |
| Long press | Confirm / HID arm payload if configured |
| Double tap | Back / cancel gesture |

Try **`01_Basics/BOOT_Button`** example to learn gestures in isolation.

---

## Serial Monitor

Connect USB, **115200 baud**. Boot banner prints SSID, password, and URL.

Optional USB CLI: set `CLI_SERIAL_ENABLED` to `1` in `config/app_registry.h` and reflash.  
Default release build is **web-first** (CLI off).

Details: [SERIAL_AND_LOGGING.md](SERIAL_AND_LOGGING.md).

---

## Custom code

Use **File → Examples → HackCard** — 50+ sketches with commented headers.  
Index: [EXAMPLES.md](../library/HackCard/EXAMPLES.md).

---

## Help

| Topic | Document |
|-------|----------|
| Arduino board settings | [ARDUINO_SETUP.md](ARDUINO_SETUP.md) |
| All documentation | [docs/README.md](../../docs/README.md) |

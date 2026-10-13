# Getting started

This guide assumes you have a **HackCard ESP32-S3** board and a **USB-C** cable.

## What you get

| Component | Purpose |
|-----------|---------|
| **Full firmware** | Wi-Fi AP + **Lab Console** at `http://192.168.4.1/` |
| **Arduino library** | 50+ examples to learn NFC, Wi-Fi, BLE, HID, LEDs without the web UI |
| **Web UI source** | Edit HTML/JS/CSS, sync into firmware |

You do **not** need a microSD card for the dashboard — the UI is embedded in firmware.

---

## Step 1 — Install tools (one time)

1. [Arduino IDE 2.x](https://www.arduino.cc/en/software)
2. **Board support:** Arduino IDE → **File → Preferences → Additional boards manager URLs**  
   Add Espressif ESP32 URL (see [Arduino setup](../HackCard_ESP32/docs/ARDUINO_SETUP.md)), then install **esp32** in Board Manager.
3. **Libraries** (Library Manager): Adafruit NeoPixel, ArduinoJson, Adafruit PN532, Adafruit BusIO.

Full board settings table: [ARDUINO_SETUP.md](../HackCard_ESP32/docs/ARDUINO_SETUP.md).

---

## Step 2 — Personalize (recommended)

Open [`HackCard_ESP32/config/user_config.h`](../HackCard_ESP32/config/user_config.h):

| Setting | Why change it |
|---------|----------------|
| `USER_NAME`, email, links | Profile / vCard / Business mode |
| `AP_SSID`, `AP_PASSWORD` | Wi-Fi network name clients join |
| `LAB_MODE_PIN` | Unlocks full Lab Console (default is demo — **change before sharing**) |

Save the file.

---

## Step 3 — Flash firmware

1. Open **`HackCard_ESP32/HackCard_ESP32.ino`** in Arduino IDE.
2. **Tools** (minimum):
   - Board: **ESP32S3 Dev Module**
   - **USB CDC On Boot: Enabled**
   - **USB Mode: USB-OTG (TinyUSB)** (needed for USB HID lab)
   - **Partition Scheme:** custom / Huge APP — use project `partitions.csv`
3. Select the correct **COM port**, click **Upload**.
4. Open **Serial Monitor @ 115200** — you should see SSID, password, and `http://192.168.4.1/`.

---

## Step 4 — Connect and open the Lab Console

1. On phone or laptop, join the HackCard Wi-Fi (2.4 GHz only — disable “5 GHz only” on some phones).
2. Browser: **`http://192.168.4.1/`**
3. **Business mode:** contact profile first; enter **Lab PIN** (from `user_config.h`) to unlock all labs.
4. Optional: **Terminal** page mirrors USB serial log.

Detailed UI tour: [Lab Console user guide](lab-console-user-guide.md).

---

## Step 5 — Optional microSD (Full Mode)

- **Flash Mode:** everything works; **Export to SD** buttons stay off until a card is present.
- **Full Mode:** insert FAT32 microSD; exports go to `/wifi/exports/`, `/nfc/exports/`, etc.

See [STORAGE.md](../HackCard_ESP32/docs/STORAGE.md).  
Do **not** copy the web UI to SD `/www/` — firmware ignores it.

---

## Next steps

| Goal | Link |
|------|------|
| Learn one hardware feature in isolation | [EXAMPLES.md](../HackCard_ESP32/library/HackCard/EXAMPLES.md) |
| Build your own app | [CUSTOM_DEVELOPMENT.md](../HackCard_ESP32/docs/CUSTOM_DEVELOPMENT.md) |
| Fix Wi-Fi visibility | [WIFI_TROUBLESHOOTING.md](../HackCard_ESP32/docs/WIFI_TROUBLESHOOTING.md) |

---

## Troubleshooting quick checks

| Symptom | Check |
|---------|--------|
| No COM port | USB cable data-capable; driver; **USB CDC On Boot** |
| Cannot see Wi-Fi | 2.4 GHz scan; SSID from Serial boot banner |
| Old dashboard after update | Reflash firmware (UI is embedded, not from SD) |
| HID lab does nothing | **USB-OTG TinyUSB**; plug into PC; focus a text field |

# Flash firmware

Upload **HackCard_ESP32.ino** and confirm boot over Serial.

---

## Before upload

- Completed [Install tools](first-setup.md) board settings
- Optional but recommended: [Personalize](personalize.md) `user_config.h`
- Correct **COM port** selected (unplug/replug if unsure)

---

## Upload steps

1. Open **`HackCard_ESP32/HackCard_ESP32/HackCard_ESP32.ino`**
2. **Sketch → Upload** (or Ctrl+U)
3. Wait for “Done uploading”
4. Open **Serial Monitor** → **115200** baud
5. Press **RESET** on the board if the log is empty

---

## Expected Serial output

You should see boot messages, peripheral init, and Wi-Fi AP startup. Typical lines include:

```text
[boot] Starting RGB/buzzer after Wi-Fi settle time...
```

At the end of boot, note the **SSID**, **password**, and dashboard URL printed by firmware (defaults come from `user_config.h`):

| Setting | Typical default |
|---------|-----------------|
| SSID | `HackCard-Setup` |
| Password | (your `AP_PASSWORD`) |
| Lab Console | `http://192.168.4.1/` |

!!! tip "Quiet serial?"
    With **USB CDC On Boot**, some hosts prefer the web **Terminal** in Lab mode. Serial should still show the boot banner at least once after reset.

---

## Quick hardware check (optional)

| Check | Pass signal |
|-------|-------------|
| Ring | Boot animation or status colors |
| Wi-Fi | SSID visible on phone (2.4 GHz scan) |
| Browser | After joining AP, `http://192.168.4.1/` loads |

---

## If upload fails

| Symptom | Try |
|---------|-----|
| Port disappears | Different USB cable/port; install CP210x/CH340 driver if needed |
| “Failed to connect” | Hold **BOOT**, tap **RESET**, release BOOT; upload again |
| Wrong partition | **Custom partition table** + `partitions.csv` in sketch folder |
| Compile errors | Install all four libraries from [First setup](first-setup.md) |

More: monorepo `HackCard_ESP32/HackCard_ESP32/docs/ARDUINO_SETUP.md` and `HackCard_ESP32/docs/WIFI_TROUBLESHOOTING.md`.

---

## Next step

→ [Open the Lab Console](lab-console.md)

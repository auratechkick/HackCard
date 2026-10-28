# Serial lab console

The RP2350 **interface is USB serial** at **115200 baud** — not a browser. Think of it as a text Lab Console.

Full command reference in firmware: `HackCard_RP2350/docs/SERIAL_CONSOLE.md`.

---

## Connect

1. USB-C to your PC (same cable carries **HID** to that PC).
2. Arduino **Serial Monitor** or any terminal → **115200**, newline.
3. Type **`help`**.

---

## Essential commands

| Command | Action |
|---------|--------|
| **`help`** | List all commands |
| **`status`** | MCU, NFC, SD, owner from config |
| **`nfc`** | Read one tag (UID / type) |
| **`watch`** | Toggle continuous NFC reads |
| **`url https://…`** | Write NDEF URL to a tag you own |
| **`text hello`** | Write NDEF text |
| **`vcard`** | Write contact from `user_config.h` |
| **`sd`** | Mount SD, create lab folders, list root |
| **`hid hello`** | After ~3 s, type text on **this PC** |
| **`os win` / `os mac` / `os linux`** | OS profile for shortcuts |
| **`site https://…`** | Open URL via OS shortcut |
| **`notes`**, **`lock`**, **`shot`** | Notepad/notes, lock, screenshot on **this PC** |
| **`log`** | Recent activity (and `/logs/activity.log` if SD present) |
| **`rgb idle\|scan\|ok\|err\|off`** | Ring pattern |
| **`beep`** | Buzzer chime |

!!! warning "HID targets your PC"
    `hid`, `lock`, `site`, etc. only affect the computer HackCard is **plugged into**. Use your own machine and stay in focus during countdowns.

---

## User button (main firmware)

| Gesture | Typical action |
|---------|----------------|
| **Short** | One NFC read |
| **Long** | Success tone + green ring |
| **Double** | LEDs off |

**BOOT** and **RUN** are hardware reset/boot — not used as lab GPIO.

---

## microSD layout (when mounted)

Firmware and **`sd`** command create paths such as:

| Path | Purpose |
|------|---------|
| `/hackcard` | Config / misc |
| `/nfc` | Dumps and exports |
| `/payloads` | Payload files |
| `/logs` | Activity log |
| `/ducky` | Ducky Script files for HID example |

---

## No web dashboard

If you need **`http://192.168.4.1/`**, Wi-Fi labs, or BLE — use **HackCard ESP32-S3** and the **[ESP32 user guide](https://auratechkick.github.io/HackCard/esp32/)**.

---

## Next step

→ [Default PINs & serial](default-credentials.md) · [Arduino examples](arduino-examples.md)

# Default Wi-Fi, PINs, and URLs

Out of the box, firmware uses the values in **`HackCard_ESP32/config/user_config.h`**. Use these for your **first login** — then change them before sharing the board.

!!! danger "Change before demos, resale, or classroom handout"
    Defaults are public in this repository. Anyone nearby can join your AP or unlock Lab mode if you keep factory values.

---

## Lab Console (Wi-Fi)

| Setting | Default value | Where to change |
|---------|---------------|-----------------|
| **Wi-Fi SSID (AP)** | `HackCard-Setup` | `AP_SSID` in `user_config.h` or Lab Console **Settings** (saved to flash) |
| **Wi-Fi password** | `hackcard2026` | `AP_PASSWORD` (minimum 8 characters) |
| **Lab Console URL** | `http://192.168.4.1/` | Fixed AP gateway (use **http**, not https) |
| **Lab PIN** (unlock Lab mode + sensitive actions) | `1234` | `LAB_MODE_PIN` in `user_config.h` or **Settings** after unlock |
| **Vault PIN** (where vault features use it) | `0000` | `VAULT_PIN` in `user_config.h` |

---

## USB serial

| Setting | Value |
|---------|--------|
| Baud rate | **115200** |
| Boot banner | Prints SSID, password, and dashboard URL |

---

## Business profile placeholders

Until you edit `user_config.h`, profile fields are generic, for example:

| Field | Default placeholder |
|-------|---------------------|
| Name | `Your Name` |
| Email | `you@example.com` |
| Company | `Your Company` |

Example sketches that write vCard or URLs often use **`https://auratechkick.github.io/HackCard/`** as a demo link — edit the **EDIT ME** section at the top of each sketch for your own site.

---

## Arduino example sketches

Many examples that ask for a **lab PIN** use the same default as firmware:

| Context | Default PIN in examples |
|---------|-------------------------|
| NFC erase / clone / kill labs | **`1234`** (must match your `LAB_MODE_PIN` if you changed firmware) |

Always read the **EDIT ME** block at the top of the sketch you upload.

---

## Quick first-time login

1. Flash firmware ([First program](first-program.md)).
2. Join Wi-Fi: **`HackCard-Setup`** / **`hackcard2026`**.
3. Browser: **`http://192.168.4.1/`**.
4. Enter Lab PIN: **`1234`** to open full labs.
5. Open **Settings** → set new AP password and Lab PIN → save.

To reset saved flash settings back to `user_config.h`, use example **`07_System/Factory_Reset`**, then re-flash main firmware.

# Default PINs & serial

RP2350 has **no Wi-Fi password** and **no Lab Console URL**. Use these defaults for your first session.

---

## Serial connection

| Setting | Value |
|---------|--------|
| **Baud rate** | **115200** |
| **Line ending** | Newline (NL or CR+LF) |
| **First command** | **`help`** |

---

## PINs and placeholders

| Item | Default | Where to change |
|------|---------|-----------------|
| **Lab PIN** | **`1234`** | `LAB_MODE_PIN` in `user_config.h` |
| Profile name | `Your Name` | `USER_*` fields in `user_config.h` |
| Email | `you@example.com` | same |

Many **Arduino examples** that ask for a lab PIN use **`1234`** unless you changed firmware.

!!! danger "Change before sharing the board"
    Treat **`1234`** as a public demo value. Set your own `LAB_MODE_PIN` before classroom or resale.

---

## Example sketch demo URLs

Examples often use **`https://auratechkick.github.io/HackCard/rp2350/`** or **`https://auratechkick.github.io/HackCard/`** in **EDIT ME** blocks — replace with your own links.

---

## What is *not* on RP2350

| ESP32 default | RP2350 |
|---------------|--------|
| Wi-Fi `HackCard-Setup` / `hackcard2026` | **N/A** |
| `http://192.168.4.1/` | **N/A** |
| Web Lab PIN unlock | Serial + `user_config.h` only |

ESP32 defaults: [ESP32 guide — Default Wi-Fi & PINs](https://auratechkick.github.io/HackCard/esp32/getting-started/default-credentials/).

---

## Reset saved SD data

**`06_System/Factory_Reset`** removes SD lab folders; **`user_config.h`** is unchanged until you re-flash after editing it.

Steps: [Next steps — Factory reset](next-steps.md).

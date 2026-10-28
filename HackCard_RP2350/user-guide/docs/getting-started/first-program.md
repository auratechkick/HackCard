# Flash firmware

Upload **`HackCard_RP2350.ino`** and verify the serial lab.

---

## Before upload

- [First setup](first-setup.md) board settings (**Generic RP2350**, **2MB (no FS)**, **Pico SDK**)
- Optional: [Personalize](personalize.md) `user_config.h`
- Serial Monitor: **115200**, line ending **Newline** (NL or CR+LF)

---

## Upload steps

1. Open **`HackCard_RP2350/HackCard_RP2350/HackCard_RP2350.ino`**
2. Select the correct **COM port**
3. **Upload**
4. Open **Serial Monitor @ 115200**
5. Press **RESET** if the banner is missing

---

## Expected result

Boot text should show board name, firmware **0.1.0**, and PN532/SD status. Then a prompt for commands, for example:

```text
hackcard>
```

Type:

```text
help
```

You should see a command list (`nfc`, `vcard`, `sd`, `hid`, …).

---

## Quick checks

| Check | Pass signal |
|-------|-------------|
| Serial | `help` prints commands |
| NFC | `nfc` with a tag near the antenna prints UID |
| HID | `hid hello` types on **this PC** after countdown (focus a text field) |
| Button | Short press triggers one NFC read in main firmware |

---

## If upload fails

| Symptom | Try |
|---------|-----|
| Wrong chip profile | **2MB (no FS)**, not 4 MB |
| HID/Keyboard compile errors | **USB Stack: Pico SDK** |
| No COM port | Data cable; hold **BOOT** while plugging in if needed |

---

## Next step

→ [Serial lab console](serial-lab.md)
